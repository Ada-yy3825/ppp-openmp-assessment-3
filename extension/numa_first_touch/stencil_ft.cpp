#include "../../core/stencil.h"

#include <cstdio>

#include <cstdlib>

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

            for (std::size_t k = 1; k < NZ - 1; ++k) {

                u_next[idx(i, j, k)] =

                    (u[idx(i - 1, j, k)] + u[idx(i + 1, j, k)] +

                     u[idx(i, j - 1, k)] + u[idx(i, j + 1, k)] +

                     u[idx(i, j, k - 1)] + u[idx(i, j, k + 1)]) /

                    6.0;

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

static void init_first_touch(double* u)

{

#pragma omp parallel for default(none) shared(u) schedule(static)

    for (std::size_t i = 0; i < NX * NY * NZ; ++i) {

        u[i] = 0.0;

    }

#pragma omp parallel for default(none) shared(u) schedule(static)

    for (std::size_t j = 0; j < NY; ++j) {

        for (std::size_t k = 0; k < NZ; ++k) {

            u[idx(0, j, k)] = 1.0;

        }

    }

}

int main()

{

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

    init_first_touch(a);

    init_first_touch(b);

    for (int s = 0; s < NSTEPS; ++s) {

        jacobi_step(a, b);

        std::swap(a, b);

    }

    std::printf("checksum = %.6e\n", checksum(a));

    std::free(a);

    std::free(b);

    return 0;

}