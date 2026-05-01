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

I parallelised the outer two loops of the 3D Jacobi update using `#pragma omp parallel for collapse(2) schedule(static)`. The innermost `k` loop is kept sequential within each thread to preserve contiguous memory access and make the loop SIMD-friendly. Each timestep reads from one buffer and writes to the other buffer, so there is no in-place update race. After every timestep, I swap the two pointers using `std::swap(a, b)`. Both buffers are initialised before the time loop, so every timestep has a valid read source and write destination.

## Section 2 — Strong-scaling curve

Describe the shape of your speedup curve across `{1, 16, 64, 128}` threads (1 = serial, 16 = one NUMA domain, 64 = one socket, 128 = full node). At which thread count does it first depart significantly from linear? Which hardware boundary explains the departure (CCD L3, socket memory bandwidth, cross-socket interconnect)? Minimum 50 words.

The scaling curve is not linear across the whole thread ladder. The 16-thread run is slower than the 1-thread baseline in my measurements, which suggests that the overhead of creating and coordinating the OpenMP team, plus memory-placement and bandwidth effects, dominates at that size. Scaling improves substantially at 64 and 128 threads, where the stencil can use more of the socket and full-node memory bandwidth. The main departure from ideal scaling happens immediately at 16 threads, and the later behaviour is best explained by the memory-bound nature of the stencil, socket bandwidth limits, and contention across Rome’s NUMA domains.

## Section 3 — Extension choice and why

Which extension did you pick (`numa_first_touch` / `false_sharing` / `simd`)? Why was it the right target for *this* kernel on *this* machine? Minimum 50 words.

I chose the `numa_first_touch` extension because the Jacobi stencil is strongly memory-bound on Rome. The kernel performs little computation per byte loaded, so memory placement is more important than extra arithmetic throughput. Rome has eight NUMA domains, and remote memory access can be much slower than local DRAM access. Parallel first-touch is therefore a natural target: it controls where the pages are physically placed and helps each thread access memory local to its NUMA domain during the stencil sweep.

## Section 4 — Extension mechanism and measured delta

Explain *how* your extension changes the code and *why* that helps on Rome hardware. Quote your measured before/after numbers (these come from your own self-benchmarks; CI checks they're internally consistent with what your `EXTENSION.md` header reports). If the delta was small, explain what dominated. Minimum 50 words.

The extension compares a naive version using single-threaded initialisation with a first-touch version using parallel initialisation. Both versions allocate memory with `posix_memalign` so allocation itself does not initialise the data. In the naive version, the master thread writes the arrays first, so most pages are placed on one NUMA domain. In the first-touch version, OpenMP threads initialise different parts of the arrays using the same static traversal pattern as the compute loop. My measured before/after times were `before_time_s = 18.659814450680003` and `after_time_s = 1.937264186`, giving `delta_percent = 89.62%`. This large improvement is consistent with avoiding remote NUMA traffic on Rome.

## Section 5 — Counterfactual on different hardware

If you were running this on an Ice Lake node (2-NUMA-domain, 64 core, higher per-core bandwidth) instead of Rome (8 domains, 128 core), would your extension still help, harm, or be neutral? Why? Minimum 50 words.

On an Ice Lake node with fewer NUMA domains, the `numa_first_touch` extension would probably still help, but the benefit would likely be smaller than on Rome. Rome has eight NUMA domains, so poor placement can cause many threads to read remote memory across NUMA boundaries. A two-domain Ice Lake system has a simpler memory topology and less opportunity for extreme page-placement imbalance. If the working set is still larger than cache and the program uses both sockets, first-touch remains useful. On a single-socket or single-NUMA system, the extension would mostly be neutral.

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain what your extension changes about data layout or work distribution, and why it matters specifically on Rome (as opposed to a single-socket or single-NUMA machine).**

The NUMA first-touch extension changes how memory pages are placed before the stencil loop begins. Instead of one master thread initialising all arrays, each OpenMP thread writes part of the grid using the same static traversal pattern as the compute loop. On Rome, this matters because the node has eight NUMA domains. Linux places each page on the NUMA node of the thread that first writes it, so parallel first-touch distributes pages across domains and reduces remote DRAM access. On a single-NUMA machine, this placement effect would be much less important.
