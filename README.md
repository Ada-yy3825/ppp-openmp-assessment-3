# Assignment 3 — 3D Jacobi stencil (core + extension)

This is the **starter repo** for Assignment 3 of the PPP-OpenMP assessment. **Target: 40 marks** (out of 100 across A1+A2+A3) — 25 core + 15 extension.

A 7-point 3D Jacobi stencil, split into a **core** implementation (25 pts) and a **chosen extension** (15 pts).

## Split structure

- `core/` — parallelise the stencil step across `{1, 16, 64, 128}` threads on Rome (1 = serial, 16 = one NUMA domain, 64 = one socket, 128 = full node).
- `extension/numa_first_touch/` — demonstrate parallel first-touch init and measure the NUMA delta.
- `extension/false_sharing/` — demonstrate cache-line padding at per-thread accumulator boundaries.
- `extension/simd/` — demonstrate `#pragma omp simd` on the innermost loop.

You must implement **core** and **exactly one** extension. Declare your choice in `EXTENSION.md`'s structured header.

## What you have

- `core/stencil.h`, `core/stencil.cpp` — scaffolding for the core kernel.
- `extension/*/README.md` — per-branch hints + what CI measures.
- `EXTENSION.md` — structured-header declaration of your chosen branch.
- `questions.md`, `answers.csv`, `REFLECTION.md`, `tables.csv` — deliverables.
- `evaluate.pbs` — Rome perf harness (run with `qsub evaluate.pbs` from your CX3 account).

## Build (laptop)

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18    # any Clang ≥ 16 will do
cmake --build build -j
OMP_NUM_THREADS=4 ./build/stencil
```

The CMake foreach over `extension/{numa_first_touch,false_sharing,simd}/*.cpp` auto-discovers any extension source you add — drop a `.cpp` in your chosen extension dir and re-run `cmake -B build` to register the target.

Recommended toolchain:
- macOS: `brew install llvm libomp cmake ninja`; use `/opt/homebrew/opt/llvm/bin/clang++`.
- Ubuntu / WSL: `apt install clang-18 libomp-18-dev cmake ninja-build`.

## What to do

1. Parallelise `jacobi_step()` in `core/stencil.cpp`.
2. Pick ONE extension branch. Read its README. Implement.
3. Declare your choice in `EXTENSION.md` and fill in `before_time_s` / `after_time_s` / `delta_percent` from your CI-measured runs.
4. Answer `questions.md` in `answers.csv`.
5. Fill in `REFLECTION.md` (it has separate sections for core and extension).

## Roofline target

A3 is **memory-bound** (OI ≈ 0.14 FLOPs/byte). The grader compares your achieved bytes/s against measured STREAM bandwidth at the thread count you ran: 246.2 GB/s ceiling (32 threads, one-per-CCX recipe), 231.5 GB/s at 128 threads full-node, 116.0 GB/s on one socket. Graduated thresholds: 0.70 / 0.50 / 0.30 / 0.15 of measured STREAM. See `docs/outcomes.md` in the lectures repo for the full scale.

## Rubric (40 pts = 25 core + 15 extension)

### Core (25 pts)

| Component | Pts | How measured |
|---|---|---|
| Build + TSan clean | 2 | Clang-18 + TSan + Archer |
| Correctness (graduated, per thread count) | 8 | `smart_diff.py` on the output checksum at `{1, 16, 64, 128}`; 2 pts each |
| Roofline performance (memory-bound) | 6 | Achieved bytes/s vs measured STREAM at 128T on Rome; correctness-gated |
| `tables.csv` internal consistency | 2 | Per-row `speedup = T(1)/T(P)` and `efficiency = speedup/P` within 2 %. No canonical cross-check. |
| Style | 2 | Lint |
| MCQ | 2 | Auto-grading |
| REFLECTION.md format + completion | 1 | CI-format-check (no canonical numerical cross-check) |
| Reasoning question (instructor-marked) | 2 | Manual 0/1/2 |

### Extension (15 pts — student picks one of three)

| Component | Pts |
|---|---|
| Implementation (code builds + correctness + the mechanism is actually used) | 7 |
| Measured delta — soft threshold per extension type | 5 |
| Reasoning question (instructor-marked) | 3 |

**Soft delta thresholds:**

- NUMA first-touch: full marks if `delta_percent ≥ 15`; half marks if `≥ 5`; attempt credit otherwise.
- False sharing: same thresholds.
- SIMD: full marks if `before/after ≥ 1.2×`; half marks if `≥ 1.05×`.

## EXTENSION.md header

```
---
chosen: numa_first_touch        # one of: numa_first_touch | false_sharing | simd
before_time_s: 4.52
after_time_s: 2.81
delta_percent: 37.8
---
```

The header is checked for *internal* consistency (delta_percent must be consistent with before_time_s / after_time_s within ±10 %). Canonical re-run timings are not compared against your reported numbers for grading.

## Grading model

Grading is run once by the instructor at the **end of day 5** on a canonical CX3 Rome node. **All performance is correctness-gated** — any thread-count correctness fail zeros the perf component. No LLM is used in the summative grading path.

## What the formative CI checks (every push)

- **Build + correctness** of the core stencil at `{1, 2, 4, 8, 16}` threads under Clang-18 + ThreadSanitizer + Archer OMPT.
- **Extension build + correctness** if `EXTENSION.md` picks one and the source is present.
- **Lint**: clang-format, clang-tidy, cppcheck.
- **REFLECTION format**: required headers (Section 1–5 + Reasoning question) present + each section ≥ 50 words.
- **Language check**: only English in Markdown and C++ comments.
- **No committed build artifacts**: `.o` / executables / `build/` are rejected.

There is **no Rome benchmark CI on student forks**. Performance is measured by the instructor at the end of the cohort on a CX3 Rome node. You can run `evaluate.pbs` on your own CX3 account to populate your `tables.csv` and the before/after evidence for `EXTENSION.md`.

## What you may NOT do

- Do not rename source files or public function signatures.
- Do not add new headers / dependencies / third-party libraries.
- Do not modify `.github/workflows/` (overwritten at grading time).
- Do not touch `bin/smart_diff.py` or other harness files.

If you need to change something not covered above, ask first.

## Useful pointers

- Lectures repo (slides, snippets, brief, rubric): https://github.com/ese-ada-lovelace-2025/ppp-openmp
- OpenMP 5.1 spec: https://www.openmp.org/spec-html/5.1/openmp.html
- Imperial CX3 docs: https://imperialcollegelondon.atlassian.net/wiki/spaces/HPC/

## Assessment timeline

Brief released day 2 morning. A3 is completable by **end of day 4**. Day 5 final snapshot is graded.
