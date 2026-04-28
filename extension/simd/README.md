# Extension: SIMD on the innermost loop

Demonstrate that `#pragma omp simd` (plus alignment hints) produces measurable vectorisation gains on the innermost stencil loop.

## What to deliver

Two variants, both in this directory:

- `stencil_scalar.cpp` — stencil compiled **without** the `#pragma omp simd` annotation (rely only on autovectorisation).
- `stencil_simd.cpp` — stencil with `#pragma omp simd aligned(u, u_next: 64) safelen(8)` on the innermost (k) loop, plus any `aligned_alloc`-style allocation required to honour the alignment assertion.

Both compile to separate binaries.

## Why the before/after may be small

Modern compilers (Clang 18, GCC 13) autovectorise simple stencils well without pragmas. The `simd` pragma typically helps by:

- Asserting pointer alignment, enabling aligned loads/stores (→ higher throughput).
- Asserting `safelen(N)`, giving the vectoriser permission to use wider vectors without proving independence.
- Documenting intent (helps compilers that are conservative about aliasing).

Your job is to *measure* the difference and *explain* it — a modest delta with a clear analysis (Bloom: Analyze) outscores a large unexplained one.

## Scoring specifics

- Full marks (5/5): `before/after ≥ 1.2×` (i.e. ≥ 20% speedup from SIMD).
- Half marks: `≥ 1.05×`.
- Attempt credit: below that, with the result grounded in your measurements.

## What the grader reads

- `perf-results-a3-ext.json` — CI times for both variants at 128 threads on Rome.
- `EXTENSION.md` — your declared times + ratio (reported as `delta_percent` = `(before - after) / before × 100`, so a 1.2× speedup corresponds to ~16.7%).

## Reading list

- Day 4 slide deck — SIMD + alignment section.
- `snippets/day4/simd_axpy.cpp` in the lectures repo — demonstrates the `aligned(...: 64) safelen(8)` idiom.
