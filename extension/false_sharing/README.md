# Extension: false-sharing mitigation

Demonstrate that padding per-thread accumulators to a full cache line eliminates false sharing and improves scaling.

## What to deliver

Two variants, both in this directory:

- `stencil_packed.cpp` — stencil augmented with an auxiliary per-thread scalar (e.g. a thread-local residual accumulator) packed adjacently on 8-byte boundaries.
- `stencil_padded.cpp` — identical stencil but with the per-thread scalars padded to 64-byte alignment (e.g. `alignas(64)`).

Both compile to separate binaries. CI runs both at 128 threads on Rome and compares times.

## Why this tests false sharing

The stencil itself is unlikely to exhibit false sharing (writes are to distinct grid points). You must *introduce* a per-thread accumulator to make the false-sharing case visible — for instance, track the thread-local max residual after each sweep:

```cpp
struct Packed  { double residual; };                    // 8 B, adjacent
struct alignas(64) Padded { double residual; };          // one cache line each
```

With 128 threads each writing to `accums[tid].residual` on every sweep, the packed version ping-pongs cache lines between cores. The padded version does not.

## What the grader reads

- `perf-results-a3-ext.json` — CI times for both variants.
- `EXTENSION.md` — your declared `before_time_s` (packed) / `after_time_s` (padded) / `delta_percent`.

## Reading list

- Day 4 slide deck — false-sharing live demo section.
- `snippets/day4/false_sharing_demo.cpp` in the lectures repo — minimal packed-vs-padded demo with a doctest.
