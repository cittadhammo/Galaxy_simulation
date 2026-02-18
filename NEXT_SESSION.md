# Next Session Plan (Step by Step)

This project is now on branch:

`refactor/headless-physics-split`

Use this file tomorrow as a checklist.

## 0) Open terminal in repo

```bash
cd /home/cittadhammo/Github/Galaxy_simulation
```

## 1) Confirm branch

```bash
git branch --show-current
```

Expected:

`refactor/headless-physics-split`

If you are not on that branch:

```bash
git checkout refactor/headless-physics-split
```

## 2) (Optional but recommended) push branch to GitHub once

This keeps the branch safe remotely:

```bash
git push -u origin refactor/headless-physics-split
```

## 3) Build and run baseline before refactor

```bash
bash unix_run.sh
bash run_sim.sh
```

Confirm the simulation starts normally before code changes.

## 4) Create a safety checkpoint commit (if you have local edits)

Check status:

```bash
git status --short
```

If files are modified and you want a checkpoint:

```bash
git add -A
git commit -m "chore: checkpoint before physics-render separation"
```

## 5) Refactor scope for tomorrow (only first incremental step)

Goal for tomorrow:

1. Add `SimulationConfig` and `SimulationState` data structures.
2. Move star arrays from `Computer` static globals into `SimulationState`.
3. Change `Computer::init` and `Computer::compute` to use these structs.
4. Change `Renderer::init_vbo` / `Renderer::update_vbo` to read from `SimulationState`.
5. Keep behavior identical (no networking, no second executable yet).

## 6) Validation after that step

Run:

```bash
bash unix_run.sh
bash run_sim.sh
bash run_batch.sh 200 2 outputs/test_after_step1
```

Success criteria:

- Interactive mode still works.
- Batch mode still writes snapshots.
- No feature changes, only architecture cleanup.

## 7) Suggested commit after step 1

```bash
git add -A
git commit -m "refactor: introduce simulation state/config boundary"
```

## 8) What to paste at start of tomorrow's Codex session

Use this exact message:

`Continue from NEXT_SESSION.md, execute section 5 only, and keep changes minimal with no behavior change.`

## 9) Assistant handoff notes (for Codex)

Architecture facts already confirmed:

- Main loop and threading are in `sources/main.cpp`.
- Simulation/render coupling lives mainly in:
  - `includes/Simulator.hpp`
  - `sources/Simulator.cpp`
  - `sources/Renderer.cpp` (reads `Computer` globals directly)
  - `sources/Computer.cpp` (reads `Menu`/`Simulator` globals directly)
- Current cloud batch is not truly headless physics, because it still relies on OpenGL via `xvfb`.

Constraints for first refactor step:

- C++17+
- minimal dependencies
- no behavior change
- no network/distributed split yet
- no full rewrite

Execution rule for tomorrow:

- make only the Section 5 changes
- compile and run Section 6 checks
- commit with message from Section 7
