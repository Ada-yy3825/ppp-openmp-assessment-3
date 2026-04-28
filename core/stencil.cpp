// A3 core — STUDENT IMPLEMENTATION.
//
// 7-point 3D Jacobi stencil over an NX × NY × NZ grid for NSTEPS timesteps.
// Target: parallelise `jacobi_step`, achieve ≥ 0.5 of the memory-bound
// roofline ceiling on Rome at 128 threads (≈ 16 GFLOPs achieved of the
// ~32 GFLOPs ceiling).
//
// You may use `collapse(2)` or `collapse(3)` as appropriate. The inner loop
// should be SIMD-friendly; consider `#pragma omp simd` here or in the
// extension/simd branch.
//
// This core file contains the parallelised stencil step + checksum + main().
// The CI builds ./build/stencil and runs it at {1, 16, 64, 128} on Rome
// (1 = serial, 16 = one NUMA domain, 64 = one socket, 128 = full node).

#include "stencil.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include <vector>

inline std::size_t idx(std::size_t i, std::size_t j, std::size_t k)
{
    return (i * NY * NZ) + (j * NZ) + k;
}

void jacobi_step(const double* u, double* u_next)
{
    // TODO(student): parallelise with #pragma omp parallel for collapse(2).
    // Starter is serial so the file builds on day 2.
    for (std::size_t i = 1; i < NX - 1; ++i) {
        for (std::size_t j = 1; j < NY - 1; ++j) {
            for (std::size_t k = 1; k < NZ - 1; ++k) {
                u_next[idx(i, j, k)] =
                    (u[idx(i - 1, j, k)] + u[idx(i + 1, j, k)] +
                     u[idx(i, j - 1, k)] + u[idx(i, j + 1, k)] +
                     u[idx(i, j, k - 1)] + u[idx(i, j, k + 1)]) / 6.0;
            }
        }
    }
}

double checksum(const double* u)
{
    double s = 0.0;
    for (std::size_t i = 0; i < NX * NY * NZ; ++i) {
        s += u[i];
    }
    return s;
}

static void init(double* u)
{
    // Parallel first-touch init: each thread writes the portion it will read,
    // putting pages on the right NUMA domain.
#pragma omp parallel for default(none) shared(u)
    for (std::size_t i = 0; i < NX * NY * NZ; ++i) {
        u[i] = 0.0;
    }
    // Dirichlet BC on one face — drives the diffusion.
#pragma omp parallel for default(none) shared(u)
    for (std::size_t j = 0; j < NY; ++j) {
        for (std::size_t k = 0; k < NZ; ++k) {
            u[idx(0, j, k)] = 1.0;
        }
    }
}

int main()
{
    std::vector<double> a(NX * NY * NZ);
    std::vector<double> b(NX * NY * NZ);

    init(a.data());
    std::memcpy(b.data(), a.data(), NX * NY * NZ * sizeof(double));

    for (int s = 0; s < NSTEPS; ++s) {
        jacobi_step(a.data(), b.data());
        std::swap(a, b);
    }
    // Deterministic output — correctness channel only. Timing via hyperfine.
    std::printf("checksum = %.6e\n", checksum(a.data()));
    return 0;
}
