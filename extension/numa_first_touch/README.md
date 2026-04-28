# Extension: NUMA first-touch

Demonstrate that parallel first-touch initialisation places memory pages on the NUMA domain of the writing thread, avoiding cross-domain traffic during the stencil sweep.

## What to deliver

Two variants, both in this directory:

- `stencil_naive.cpp` — stencil with **single-threaded** initialisation of `u` / `u_next`. All pages end up on one NUMA domain; the 128-thread run pays cross-node bandwidth penalties.
- `stencil_ft.cpp` — identical stencil but with **parallel first-touch** initialisation: each thread writes the region it will later read.

Both compile to separate binaries. CI runs both at 128 threads on Rome, computes `delta_percent = (naive_time - ft_time) / naive_time × 100`, and expects a substantial positive delta (≥ 15% for full marks, ≥ 5% for half).

## Implementation hints

1. Copy `../core/stencil.cpp` (relative to this directory) as a starting point for both variants.
2. In `stencil_naive.cpp`, replace the parallel init loop with a plain serial one.
3. In `stencil_ft.cpp`, keep (or add) `#pragma omp parallel for` over the init loops, with the **same iteration order** as the compute step. This is the whole point — traversal order at init must match the compute step to get correct page placement.
4. Both must produce the same checksum (correctness gates the delta).

## What the grader reads

- `perf-results-a3-ext.json` — CI-measured times at 128T for both variants.
- `EXTENSION.md` — your declared before/after times (CI cross-checks within 10%).

## Reading list

- Day 4 slide deck (`slides/day4.qmd`) section "NUMA first-touch on 8-domain EPYC 7742".
- `docs/rome-inventory.md` — numa-hardware output shows exactly which cores map to which NUMA domain.
- `snippets/day4/numa_first_touch.cpp` in the lectures repo — a minimal working example.
