---
chosen: numa_first_touch
before_time_s: 0.00
after_time_s: 0.00
delta_percent: 0.0
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

<!-- your rationale here -->
