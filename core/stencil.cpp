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
#include <utility>

inline std::size_t idx(std::size_t i, std::size_t j, std::size_t k)
{
    return (i * NY * NZ) + (j * NZ) + k;
}

void jacobi_step(const double* u, double* u_next)
{
#pragma omp parallel for collapse(2) default(none) shared(u, u_next) schedule(static)
    for (std::size_t i = 1; i < NX - 1; ++i) {
        for (std::size_t j = 1; j < NY - 1; ++j) {
#pragma omp simd
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
    // posix_memalign returns *uninitialised* memory aligned to 64 bytes
    // (cache line; covers AVX2's 32-byte vector loads). The first write
    // is the first touch — `init()` below is parallel, so pages distribute
    // across the team's NUMA domains. Using `std::vector<double>(N)` here
    // would value-initialise on the master thread and pre-empt our first
    // touch — see day-4 deck "First-touch — the std::vector trap".
    const std::size_t bytes = NX * NY * NZ * sizeof(double);
    void* a_raw = nullptr;
    void* b_raw = nullptr;
    if (posix_memalign(&a_raw, 64, bytes) != 0 ||
        posix_memalign(&b_raw, 64, bytes) != 0) {
        std::fprintf(stderr, "posix_memalign failed\n");
        return 1;
    }
    auto* a = static_cast<double*>(a_raw);
    auto* b = static_cast<double*>(b_raw);

    init(a);  // parallel first-touch init
    init(b);  // parallel first-touch init for second buffer

    for (int s = 0; s < NSTEPS; ++s) {
        jacobi_step(a, b);
        std::swap(a, b);
    }
    // Deterministic output — correctness channel only. Timing via hyperfine.
    std::printf("checksum = %.6e\n", checksum(a));

    std::free(a);
    std::free(b);
    return 0;
}
