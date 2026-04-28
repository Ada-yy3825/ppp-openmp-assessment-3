# A3 — Multiple-choice questions

Fill in your answers in `answers.csv` (one letter per question, A/B/C/D).

## q01

The 7-point 3D Jacobi stencil has operational intensity (FLOPs per byte of memory traffic) of approximately:

- A. > 10 (compute-bound).
- B. 1.0 (balanced).
- C. 0.14 (strongly memory-bound).
- D. Depends on problem size.

## q02

"First-touch" placement on a NUMA system means:

- A. A page of memory is allocated on the NUMA node where the thread that first writes to it runs.
- B. The first thread to touch the memory locks it for itself.
- C. Memory allocations always go to NUMA node 0.
- D. The hardware pre-fetches memory on the first access.

## q03

To avoid NUMA penalties in a 3D Jacobi kernel on a dual-socket node, initialise the grid with:

- A. A single-threaded loop, then parallelise the compute step.
- B. A `#pragma omp parallel for` loop using the same traversal order as the compute step.
- C. `calloc` — it's automatically NUMA-aware.
- D. `mmap` with `MAP_ANONYMOUS`.

## q04

False sharing occurs when:

- A. Two threads write to the same memory address concurrently.
- B. Two threads write to different addresses that happen to land on the same cache line.
- C. One thread reads before another writes — a race.
- D. The compiler reorders unrelated writes.

## q05

Imperial CX3 Rome node has 8 NUMA domains (2 sockets × 4 per socket, 16 cores per domain). For a 128-thread Jacobi run with `OMP_PLACES=cores OMP_PROC_BIND=close`, threads are:

- A. Bound to specific cores; consecutive thread IDs land on nearby cores (same CCD where possible).
- B. Allowed to migrate freely between cores.
- C. All bound to socket 0.
- D. Allowed to use hyperthreads automatically.

## q06

A student's A3-core at 128 threads achieves 35 GB/s of memory bandwidth on a Rome node where STREAM measures 230 GB/s. Their roofline fraction is approximately:

- A. 0.15 — there is substantial headroom.
- B. 0.70 — well optimised.
- C. 1.0 — at peak.
- D. 0.45 — middling.

## q07

Which synchronisation is typically *implicit* at the end of a `#pragma omp for`?

- A. `#pragma omp critical`
- B. `#pragma omp barrier`
- C. `#pragma omp flush(everything)`
- D. `#pragma omp atomic`

## q08

You add `#pragma omp parallel for` to the Jacobi step but see zero speedup at 64 threads. First-touch initialisation is correctly parallelised. The next most likely cause is:

- A. The loop body is wrong.
- B. Memory bandwidth at one NUMA domain is saturated; adding threads within that domain doesn't help.
- C. The compiler disabled OpenMP.
- D. Cache coherence is broken.

## q09

To pad a per-thread accumulator array against false sharing, the padding stride should be at least:

- A. `sizeof(double)` bytes.
- B. 32 bytes.
- C. 64 bytes (one cache line on Rome / most x86).
- D. 4096 bytes (one page).

## q10

`#pragma omp simd safelen(8)` on an inner loop declares:

- A. At least 8 consecutive iterations are safe to execute concurrently without cross-iteration data dependences.
- B. The compiler must generate 8-wide vector instructions.
- C. 8 threads will run the loop.
- D. The loop body has exactly 8 operations.

## q11

On Rome, `#pragma omp simd` on the innermost Jacobi loop most likely helps because:

- A. AVX2 lets the CPU process multiple grid points per instruction, improving compute throughput per cycle.
- B. It reduces memory bandwidth required.
- C. It disables OpenMP threading.
- D. It forces first-touch placement.

## q12

You picked the false-sharing extension and measured before/after times 4.8 s and 3.2 s. Your `delta_percent` field should read:

- A. 33.3%
- B. 50%
- C. 1.5
- D. -33.3%

## q13

If a kernel's OI is 0.14 and STREAM is 230 GB/s, the bandwidth-limited GFLOPs ceiling is approximately:

- A. 230
- B. 32
- C. 1600
- D. 0.47

## q14

Between two successive Jacobi timesteps, you must not read the output of the current step while it's still being computed. The most common way to enforce this is:

- A. Double-buffering — read from grid A, write to grid B, swap pointers between steps.
- B. `#pragma omp atomic` on every grid update.
- C. `#pragma omp critical` around the whole step.
- D. `#pragma omp flush` after every write.

## q15

If your A3 extension is SIMD but the measured `after_time_s / before_time_s` delta is only 1.02×, what should your `EXTENSION.md` report?

- A. Mark it as 1.5× — the compiler probably vectorised anyway.
- B. Leave `delta_percent` blank.
- C. Report the measured 2% delta; analyse in REFLECTION.md why vectorisation didn't help (e.g. auto-vectorised baseline, memory-bound inner loop, or alignment issue).
- D. Skip the extension and resubmit a different one.
