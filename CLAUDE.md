# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Purpose

Student assessment starter repo for **Assignment 3 — 3D Jacobi stencil** (core + one extension chosen from NUMA first-touch / false-sharing / SIMD) in the "Shared Memory Programming with OpenMP" course (2026 refresh). 40 marks (25 core + 15 extension).

Sibling repos for the other two assignments (released on a staged schedule):

- `ppp-openmp-assessment-1` — numerical integration (parallel-for + reduction + schedule choice)
- `ppp-openmp-assessment-2` — Mandelbrot two-variant

**Public** companion: lectures repo `~/projects/ppp-openmp` (slides, snippets, brief, handouts).
**Private** companion: `~/projects/ppp-openmp-grader` (instructor only — reference solutions, MCQ keys, grader script).

Grading is summative on a CX3 login node via `grade_cohort.sh` in the private grader repo. Formative CI runs on every student push on GH-hosted Ubuntu (no secrets, no self-hosted runner). **Students modify only the designated C++ source files** — do not rename files, add new headers/libraries, or modify `.github/workflows/` (overwritten at grading time).

## Build (laptop)

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18    # or equivalent Clang ≥ 16
cmake --build build -j
```

Produces:

- `build/stencil` — A3 core
- `build/ext_<name>_<file>` — A3 extension, auto-built if the student adds source under `extension/<name>/`

The CMake `file(GLOB)` foreach over `extension/${ext}/*.cpp` is reconfigure-sensitive — students adding a new `.cpp` need to rerun `cmake -B build` to register the target.

## Local correctness check

`stencil` prints a single reproducible line to stdout (the correctness token). Timing is measured by the hyperfine harness into JSON, NOT printed to stdout.

```bash
OMP_NUM_THREADS=4 ./build/stencil > out.txt
python3 bin/smart_diff.py out.txt expected_output.txt
```

`smart_diff.py` does a token-wise float-tolerant diff.

## What students submit

- `core/stencil.cpp` — student parallelises `jacobi_step()`.
- `extension/<chosen>/*.cpp` — student-supplied source for exactly one extension branch.
- `questions.md` — 15 MCQ (read-only; answers auto-graded).
- `answers.csv` — student fills in `qid,answer` (A/B/C/D per row).
- `REFLECTION.md` — fixed-format reflection (Sections 1–5 + Reasoning). CI checks header presence + ≥ 50 words per required section.
- `tables.csv` — measured times + speedup + efficiency. CI checks per-row internal consistency only.
- `EXTENSION.md` — YAML-header block declaring the chosen extension branch + before/after times. Checked for *internal* consistency only (delta_percent agrees with before/after within 10 %).

## CI workflow (`.github/workflows/ci.yml` — formative only, GH-hosted Ubuntu)

A single workflow with five parallel jobs:

- **`lint`** — clang-format-20 + clang-tidy-20 + cppcheck against `.clang-format` and `.clang-tidy`.
- **`correctness`** — Clang-18 + TSan + Archer OMPT. Reads `EXTENSION.md` to pick the extension to build, then exercises the core stencil at `{1, 2, 4, 8, 16}` threads and the extension binaries at `{1, 4, 8}`. Any `^(WARNING|ERROR): ThreadSanitizer` line fails the run.
- **`reflection-format`** — formative format-only check. A3 requires Sections 1–5 + Reasoning question.
- **`language-check`** — non-English content detector on `*.md` and C++ comments.
- **`generated-files-check`** — fails if `.o`, executables, `build/`, or other artifacts are committed.

There is **no** `assessment.yml` and **no self-hosted runner**. Summative grading is run by the instructor on CX3 from the private grader repo.

## Rome perf harness (`evaluate.pbs`)

Run at canonical thread ladder **`{1, 16, 64, 128}`** on Rome (dual-socket EPYC 7742, 128 physical cores, 8 NUMA domains): produces `perf-results-a3.json` (core scaling + chosen extension at 128T).

Timer: `hyperfine --warmup 1 --min-runs 3 --export-json`. Build flags: `-O3 -march=znver2 -mavx2 -fopenmp`.

Students may run this on their own CX3 accounts to populate `tables.csv` and the before/after evidence for `EXTENSION.md`; this is encouraged but not required. The summative score uses the instructor's canonical re-run.

## Shared lint config

`.clang-format` and `.clang-tidy` are kept in lockstep with the lectures repo and the other two assessment repos. When editing, keep them identical across all four (students pre-commit on the assessment repos, grader enforces on all).

## Hardware facts (canonical constants — see lectures-repo `docs/rome-inventory.md`)

- 128 physical cores per node (2 sockets × 64 cores), SMT off
- 8 NUMA domains per node (NPS=4 per socket, 16 cores per domain)
- Peak DP 4608 GFLOPs (AVX2 FMA, theoretical); HPL-achievable 2896 GFLOPs ≈ 63 % (foss/2024a + OpenBLAS, measured 2026-04-26)
- STREAM triad ceiling 246.2 GB/s (one thread per CCX); 231.5 GB/s at 128 threads full-node; 116.0 GB/s on one socket (measured 2026-04-26 v3)
- Intra-socket node distance 12, cross-socket 32
- Canonical thread ladder `{1, 16, 64, 128}`
