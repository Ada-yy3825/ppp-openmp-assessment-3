# A3 REFLECTION

> Complete every section. CI will:
>
> 1. Verify all `## Section` headers are present.
> 2. Verify each section has **at least 50 words**.
>
> No automatic content grading: prose is read by a human and the short
> prompt is marked on a 0 / 0.5 / 1 scale. Numbers you quote do **not**
> have to match canonical re-run timings exactly — HPC variance is real.
> The `EXTENSION.md` header is checked for *internal* consistency only
> (its before/after/delta numbers must agree with each other within ±10 %);
> canonical measurements are not compared against your reported numbers
> for grading.

## Section 1 — Core parallelisation strategy

Which loops did you parallelise? Did you use `collapse`? How did you handle the double-buffer swap between timesteps? Minimum 50 words.

<!-- your answer here -->

## Section 2 — Strong-scaling curve

Describe the shape of your speedup curve across `{1, 16, 64, 128}` threads (1 = serial, 16 = one NUMA domain, 64 = one socket, 128 = full node). At which thread count does it first depart significantly from linear? Which hardware boundary explains the departure (CCD L3, socket memory bandwidth, cross-socket interconnect)? Minimum 50 words.

<!-- your answer here -->

## Section 3 — Extension choice and why

Which extension did you pick (`numa_first_touch` / `false_sharing` / `simd`)? Why was it the right target for *this* kernel on *this* machine? Minimum 50 words.

<!-- your answer here -->

## Section 4 — Extension mechanism and measured delta

Explain *how* your extension changes the code and *why* that helps on Rome hardware. Quote your measured before/after numbers (these come from your own self-benchmarks; CI checks they're internally consistent with what your `EXTENSION.md` header reports). If the delta was small, explain what dominated. Minimum 50 words.

<!-- your answer here -->

## Section 5 — Counterfactual on different hardware

If you were running this on an Ice Lake node (2-NUMA-domain, 64 core, higher per-core bandwidth) instead of Rome (8 domains, 128 core), would your extension still help, harm, or be neutral? Why? Minimum 50 words.

<!-- your answer here -->

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain what your extension changes about data layout or work distribution, and why it matters specifically on Rome (as opposed to a single-socket or single-NUMA machine).**

<!-- your answer here; 100 words max -->
