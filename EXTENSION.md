---
chosen: numa_first_touch
before_time_s:  18.659814450680003
after_time_s: 1.937264186
delta_percent: 89.62
---

<!--
FIELDS (all required, parsed deterministically):

  chosen           one of: numa_first_touch | false_sharing | simd
  before_time_s    your measured time (s) BEFORE applying the extension
                   (i.e. the baseline — naive init / unpadded accumulator / no simd)
                   CI cross-checks against its own timing within 10%.
  after_time_s     your measured time (s) AFTER applying the extension.
  delta_percent    improvement percentage = (before - after) / before * 100
                   Must be internally consistent with before_time_s and after_time_s within 1 percentage point.

Soft delta thresholds (scored separately from implementation marks):
  NUMA first-touch    full marks if delta_percent >= 15; half marks if >= 5
  False sharing       full marks if delta_percent >= 15; half marks if >= 5
  SIMD                full marks if (after/before ratio) <= 1/1.2 (i.e. >= 1.2x speedup);
                      half marks if <= 1/1.05

Implementation marks (the majority of the extension score) come from: your code building,
running correctly, and actually implementing the chosen mechanism. Honesty about a small
delta scores better than a falsified number (CI catches falsification at the cross-check step).
-->

## Rationale (≤ 200 words)

Explain why you picked this extension for this kernel on Rome, and what the mechanism is.

I picked the NUMA first-touch extension because the 3D Jacobi stencil is memory-bound on Rome. Each update performs only a small amount of arithmetic compared with the amount of grid data it reads and writes, so memory placement has a large effect on performance. Rome has eight NUMA domains, and if the arrays are first initialised by one master thread, most pages are placed on one NUMA domain. During a 128-thread stencil sweep, many threads then access remote memory, which increases latency and reduces effective bandwidth.

The extension changes the initialisation strategy. The naive version uses single-threaded initialisation, while the first-touch version uses a parallel OpenMP loop with static scheduling. Because Linux places each page on the NUMA node of the thread that first writes it, the parallel version distributes pages across NUMA domains. The compute loop then uses the same static traversal pattern, so threads tend to read and write pages close to where they were first touched.
