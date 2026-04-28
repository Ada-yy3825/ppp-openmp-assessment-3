// A3 — shared declarations for the 3D Jacobi stencil.
#pragma once

#include <cstddef>

// Grid sized so the working set (~3 × NX*NY*NZ × 8B) exceeds per-socket L3
// cache (~256 MB) and therefore lands in DRAM. This is what makes the kernel
// bandwidth-bound on Rome. Adjust only if instructed.
constexpr std::size_t NX = 256;
constexpr std::size_t NY = 256;
constexpr std::size_t NZ = 256;
constexpr int NSTEPS = 100;

// Student-authored stencil step. Reads from `u`, writes to `u_next`.
void jacobi_step(const double* u, double* u_next);

// Reference checksum — used by the grader to verify the final field.
double checksum(const double* u);
