# Repository Guidelines

## Project Structure & Module Organization
- `sources/`: C++ implementation (`main.cpp` for interactive app, `physics_main.cpp` for physics-only runner).
- `includes/`: project headers (`Simulator`, `Renderer`, `Computer`, config/state types).
- `shaders/`: OpenGL and OpenCL shader code (`shaders/compute/cl_compute_shader.cl`).
- `resources/`: icons and image assets used by the UI/demo.
- `batch_configs/` and `batch_jobs.cfg`: batch experiment inputs.
- `docs/`: architecture and VM/cloud runbooks.
- `cloud/`: helper scripts for remote/headless runs.
- `build/` and `outputs/`: local build artifacts and generated snapshots/state files.

## Build, Test, and Development Commands
- `bash unix_run.sh`: initialize submodules, configure CMake, and build in `build/`.
- `bash run_sim.sh`: run `Galaxy_simulation` (auto-builds if missing, loads `simulation.cfg` by default).
- `bash run_physics.sh --steps 2000 --state-out outputs/physics_state.bin`: run physics-only executable.
- `bash run_batch.sh 2000 4 outputs`: run batch snapshots.
- `bash run_batch.sh --batch-config batch_configs/example.batchcfg`: run jobs from a batch config file.
- Direct CMake flow (optional): `cmake -B build . && cmake --build build -j"$(nproc)"`.

## Coding Style & Naming Conventions
- Language standard: C++17 (see `CMakeLists.txt`).
- Follow existing style in touched files: tabs are common in core simulation files; braces on new lines.
- Types/classes use `PascalCase`; functions and variables use `snake_case`; constants/macros use uppercase when present.
- Keep headers in `includes/` matched with implementations in `sources/`.
- No repo-wide formatter is currently enforced; keep diffs minimal and consistent with nearby code.

## Testing Guidelines
- No automated unit test framework is configured currently.
- Validate by running at least one executable path you changed:
  - interactive: `bash run_sim.sh`
  - physics-only: `bash run_physics.sh --steps 2000`
  - batch/export: `bash run_batch.sh --batch-config batch_configs/example.batchcfg`
- For behavior changes, include before/after notes and output paths (for example `outputs/run_001/`).

## Commit & Pull Request Guidelines
- Preferred commit style in this repo: conventional prefixes like `feat:`, `fix:`, `refactor:`, `docs:`.
- Use imperative, specific subjects (example: `feat: add camera top-view config parsing`).
- PRs should include:
  - concise problem/solution summary
  - reproduction and validation commands run
  - related issue or task link
  - screenshots/GIFs for rendering or UI-visible changes.
