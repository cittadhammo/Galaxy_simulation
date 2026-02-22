# 🌌 Galaxy simulation

![Release](https://img.shields.io/badge/Release-v4.0-blueviolet)
![Language](https://img.shields.io/badge/Language-C%2B%2B-0052cf)
![Libraries](https://img.shields.io/badge/Libraries-Dimension3D_OpenCL-00cf2c)
![Size](https://img.shields.io/badge/Size-204Mo-f12222)
![Open Source](https://badges.frapsoft.com/os/v2/open-source.svg?v=103)

<br/>

This repository contains the source code of an n-body type simulation using GPU acceleration. It is able to simulate galaxies, galaxy collisions and expanding universes. It has a menu to change the settings and an interactive camera.

<br/>

⚠️ This repository contains **submodules**, add `--recurse-submodules` when cloning ⚠️

<br/>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_1.png" width="700">
</p>

<br/>

# 📋 Summary

* **[📋 Summary](#-summary)**
* **[🎥 Video](#-video)**
* **[✨ Features](#-features)**
* **[🏗️ Architecture](#%EF%B8%8F-architecture)**
* **[📘 Beginner Guide](#-beginner-guide)**
* **[🐧 Build on Linux](#-build-on-linux)**
* **[🛠️ Install](#%EF%B8%8F-install)**
	* [📦 Dependencies](#-dependencies)
	* [▶️ Build](#%EF%B8%8F-build)
	* [🚀 Run](#-run)
	* [⚙️ Janus + Performance Update](#%EF%B8%8F-janus--performance-update)
	* [🧪 Batch Experiment Workflow](#-batch-experiment-workflow)
	* [✅ Validate Optimized Runs](#-validate-optimized-runs)
	* [☁️ Cloud VM](#%EF%B8%8F-cloud-vm)
	* [🧱 Snapshot from Checkpoint Files](#-snapshot-from-checkpoint-files)
	* [🧩 Troubleshooting](#-troubleshooting)
* **[📝 Changes](#-changes)**
* **[🧪 Simulation variants](#-simulation-variants)**
* **[🗓️ Releases](#%EF%B8%8F-releases)**
* **[🧪 Tests](#-tests)**
* **[🙏 Credits](#-credits)**

<br/>

# 🎥 Video

Here is a video explaining how the algorithm works : [<u>**Simuler 1 000 000 de Galaxies 🌌**</u>](https://www.youtube.com/watch?v=dFqjqRUWCus).

<br/>

# ✨ Features

* A menu to change the settings of the simulation

* A camera that you can control with your mouse and your wheel

* It can simulate a unique galaxy :

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_2.png" width="500">
</p>

* It can also simulate a collision between two galaxies :

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_3.png" width="500">
</p>

* And it can simulate an entire expanding universe :

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_4.png" width="500">
</p>

<br/>

# 🏗️ Architecture

The project now has two runtime paths:

* `Galaxy_simulation` (existing app):
  * OpenGL renderer + UI
  * interactive and batch snapshot export
* `Galaxy_physics` (new executable):
  * physics-only compute loop
  * no rendering/window loop in the app flow
  * optional state file export for downstream rendering/replay

Current refactor status:

* Introduced internal boundaries:
  * `SimulationConfig`: simulation parameters
  * `SimulationState`: star arrays (`positions`, `speeds`, `accelerations`, `star_types`)
* `Computer` now computes from `SimulationConfig` + `SimulationState`.
* `Renderer` now reads from `SimulationState`.
* Full real-time network streaming (VM physics -> local renderer) is not implemented yet.

<br/>

# 📘 Beginner Guide

If you are new to cloud VMs and this split setup, read:

* [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)
* [`docs/VM_CHECKLIST.md`](docs/VM_CHECKLIST.md)
* [`docs/COLAB_SESSION.md`](docs/COLAB_SESSION.md)

It explains:

* what each executable does
* exact copy/paste commands for first VM setup
* how to run physics remotely
* how to copy results back locally
* common errors and fixes
* command-only VM checklist (copy/paste)
* full Google Colab session steps (copy/paste)

<br/>

# 🐧 Build on Linux

```bash
bash unix_run.sh
```

This script:

* initializes/upgrades git submodules (`--init --recursive`)
* configures CMake with `Unix Makefiles`
* uses the OpenCL include/library shipped in this repository
* builds the executable in `./build`

## 🚀 Quick Start (Makefile)

After building, use the Makefile for common workflows:

```bash
# Run interactive galaxy simulation (from simulation.cfg)
make sim

# Run physics with checkpointing (saves config + states to outputs/DIR/)
make physic DIR=my_run

# Run physics without snapshots (physics only)
make physic DIR=my_run SNAPSHOTS=0

# Generate snapshots from existing checkpoints
make snapshot DIR=my_run OUT=snap1

# Generate snapshots with elevation angle
make snapshot DIR=my_run OUT=snap1 ANGLE=45

# Generate rotating snapshots (5 degrees per checkpoint)
make snapshot DIR=my_run OUT=rotate ROT=5

# View a state file interactively
make view IN=outputs/my_run/states/physics_state_step_200.bin

# Create video from snapshots
make movie DIR=outputs/my_run/snapshots
```

#### Makefile Targets

| Target | Description |
|--------|-------------|
| `make sim` | Run interactive galaxy from `simulation.cfg` |
| `make physic` | Run physics with worker (requires `DIR`) |
| `make snapshot` | Generate snapshots from checkpoints (requires `DIR` and `OUT`) |
| `make view` | View a state file interactively (requires `IN`) |
| `make movie` | Create video from snapshots |

#### Makefile Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `DIR` | Run name (outputs/DIR/) | (required) |
| `OUT` | Snapshot subfolder name | (required for snapshot) |
| `IN` | State file for `make view` | (required for view) |
| `STEPS` | Number of physics steps | `2000` |
| `STATE_INTERVAL` | Checkpoint interval | `40` |
| `SNAPSHOTS` | Enable snapshots (0/1) | `1` |
| `CAMERA` | Camera: `top` or `isometric` | `top` |
| `BLOOM_RED` | Red bloom intensity | (from config) |
| `BLOOM_BLUE` | Blue bloom intensity | (from config) |
| `ANGLE` | Camera elevation angle (0-360) | - |
| `ROT` | Rotation increment per checkpoint | - |
| `WIDTH` | Snapshot width | `3840` |
| `HEIGHT` | Snapshot height | `2160` |
| `HEADLESS` | Use xvfb (0/1) | `1` |
| `DELAY` | Delay between renders | - |
| `SINGLE` | Run once (0/1) | - |
| `STAMP` | Add step numbers (0/1) | `0` |
| `FPS` | Video framerate | `24` |
| `DEVICE` | OpenCL device: cpu or gpu | auto |

All targets require `DIR` (use `outputs/` prefix for tab-completion). Results stored in `outputs/DIR/`.

<br/>

## 🔄 Typical Workflow

Here's a typical workflow to create galaxy simulation videos:

### Step 1: Explore and tweak parameters
```bash
make sim
```
- Explore the simulation interactively
- Tweak parameters in the GUI (careful: max ~30000 stars for CPU performance)
- When happy with the look, click one of the **Quick export** buttons:
  - **Copy to simulation.cfg** - directly updates the root config file
  - **Copy to user.batchcfg** - saves to batch config file

### Step 2: Config is ready!
```bash
# If you clicked "Copy to simulation.cfg", it's already ready!
# If you clicked "Copy to user.batchcfg", copy it:
cp batch_configs/user.batchcfg simulation.cfg

# You can also manually edit simulation.cfg to:
# - Increase stars (e.g., 100000)
# - Adjust star speeds (e.g., stars_speed=50)
```
- Explore the simulation interactively
- Tweak parameters in the GUI (careful: max ~30000 stars for CPU performance)
- When happy with the look, click **Append current SIMCFG to file** button
  - Default output: `batch_configs/user.batchcfg`
  - You can change the file path in the text field above the button

### Step 2: Update config file
```bash
# Copy the config from batch_configs/user.batchcfg to simulation.cfg
cp batch_configs/user.batchcfg simulation.cfg

# Or manually edit simulation.cfg to:
# - Increase stars (e.g., 100000)
# - Adjust star speeds (e.g., stars_speed=50)
```

### Step 3: Run physics simulation
```bash
make physic DIR=my_experiment STEPS=2000 STATE_INTERVAL=20
```
- This saves state checkpoints every 20 steps
- Runs with the config from `simulation.cfg`

### Step 4: Generate and preview snapshots
```bash
# Check the auto-generated snapshots from the worker
ls outputs/my_experiment/snapshots/

# Or regenerate with custom settings:
make snapshot DIR=outputs/my_experiment OUT=preview SNAPSHOTS=6
make view IN=outputs/my_experiment/states/physics_state_step_200.bin
```

### Step 5: Create first movie (top-down view)
```bash
make movie DIR=outputs/my_experiment/snapshots
```

### Step 6: Create rotating video (optional)
```bash
# Generate snapshots with rotation (5 degrees per checkpoint)
make snapshot DIR=outputs/my_experiment OUT=rotate ROT=5

# Create rotating video
make movie DIR=outputs/my_experiment/rotate
```

### Step 7: Try new parameters
```bash
# Edit simulation.cfg with new parameters, then:
make physic DIR=new_experiment STEPS=3000 STATE_INTERVAL=40
```

**Tips:**
- Use different `DIR` names for each experiment to keep results separate
- `ANGLE=45` gives a 45° elevated view
- `ROT=5` rotates 5° per checkpoint (use with many checkpoints)
- Use `SNAPSHOTS=0` to run physics without any rendering overhead

<br/>

# 🛠️ Install

## 📦 Dependencies

On Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  libglew-dev \
  freeglut3-dev \
  ocl-icd-opencl-dev
```

On Arch Linux:

```bash
sudo pacman -S --needed \
  base-devel \
  cmake \
  glew \
  freeglut \
  opencl-headers \
  ocl-icd \
  clinfo
```

Install at least one OpenCL implementation:

CPU fallback:

```bash
sudo pacman -S --needed pocl
```

NVIDIA GPU:

```bash
sudo pacman -S --needed opencl-nvidia
```

## ▶️ Build

If you clone for the first time:

```bash
git clone --recurse-submodules https://github.com/angeluriot/Galaxy_simulation.git
cd Galaxy_simulation
```

Then build:

```bash
bash unix_run.sh
```

For detailed Makefile commands and variables, see the [Quick Start section](#-quick-start-makefile) above.

## 🚀 Run

### 1) Interactive Viewer

Run the full renderer + UI:

```bash
./build/Galaxy_simulation
# or
bash run_sim.sh
```

`run_sim.sh` loads `simulation.cfg` automatically when no extra arguments are passed.  
Set `camera_view=top` in `simulation.cfg` for top-down startup.

Units note:

* Distances and sizes are in simulation units (dimensionless internal units).
* They are consistent within one run/config and across saved `.bin` files.
* Use them for relative/comparative measurements (for example core size vs halo size), not real-world SI mapping.

Built-in measurement tool:

* Open the menu (`F1`) and go to `Measurement Tool`.
* Enable `ruler overlay`, choose axis (`X`, `Y`, `Z`), then move Marker `A` and `B`.
* The UI shows `Distance |A-B|` in simulation units.
* Works during paused simulation and when replaying a loaded `--state-in` file.

### 2) Batch Snapshot Export

Compute then export snapshots from multiple angles:

```bash
./build/Galaxy_simulation --batch-steps 2000 --snapshots 4 --output-dir outputs
# or
bash run_batch.sh 2000 4 outputs
```

Run jobs from a batch file:

```bash
bash run_batch.sh --batch-config batch_configs/example.batchcfg
```

If `batch_jobs.cfg` exists at repo root, `bash run_batch.sh` uses it automatically.

### 3) Physics-Only Runs (No Renderer Loop)

```bash
./build/Galaxy_physics --steps 2000
# or
bash run_physics.sh --steps 2000
```

Useful options:

```bash
# load parameters from config
bash run_physics.sh --config simulation.cfg --steps 5000

# choose preferred OpenCL device
bash run_physics.sh --device cpu --steps 2000
bash run_physics.sh --device gpu --steps 2000

# write one final state file
bash run_physics.sh --steps 5000 --state-out outputs/physics_state.bin

# write periodic checkpoints
bash run_physics.sh --steps 5000 --state-out outputs/physics_state.bin --state-interval 500
```

Argument reference (`run_physics.sh` / `Galaxy_physics`):

* `--steps <int>`: number of compute iterations. Default: `2000`.
* `--config <path>`: load first valid `SIMCFG`/`BATCHCFG` line and apply matching simulation keys.
* `--state-out <path>`: write state to binary file.
* `--state-interval <int>`: checkpoint every `N` steps (`N > 0`). `0` disables interval checkpoints.
* `--progress-interval <int>`: print progress every `N` steps (`N >= 1`). Default: `20`.
* `--device <value>`: OpenCL preference.
  * accepted: `cpu`, `gpu`
  * any other value falls back to automatic device selection
* `--checkpoint-snapshots`: render one snapshot for each checkpoint write.
  * requires `--state-out` and `--state-interval > 0`
* `--checkpoint-snapshot-dir <path>`: output folder for checkpoint snapshots.
* `--checkpoint-snapshot-camera <value>`: camera for checkpoint snapshots.
  * accepted: `top`, `isometric`

Behavior notes:

* `run_physics.sh` writes no file unless `--state-out` is set.
* with `--state-interval N`, files are named like `outputs/physics_state_step_500.bin`.
* the final step is always checkpointed when interval mode is enabled.
* progress logs default to every 20 steps; override with `--progress-interval N`.

### 4) Checkpoint Snapshot Workflows

Inline snapshots from `run_physics.sh`:

```bash
bash run_physics.sh --steps 5000 --state-out outputs/physics_state.bin --state-interval 500 --checkpoint-snapshots
```

Separate async worker (recommended for clean separation):

```bash
# Terminal 1
bash checkpoint_snapshot_worker.sh --state-glob "outputs/physics_state_step_*.bin" --camera top

# Terminal 2
bash run_physics.sh --steps 5000 --state-out outputs/physics_state.bin --state-interval 500
```

One-command launcher (starts worker, runs physics, stops worker automatically):

```bash
bash run_physics_with_worker.sh -- --steps 5000 --state-out outputs/physics_state.bin --state-interval 500
```

```bash
bash run_physics_with_worker.sh \
  --worker-camera isometric \
  --worker-headless \
  --worker-snapshot-dir outputs/checkpoint_snapshots_iso \
  -- --steps 5000 --state-out outputs/physics_state.bin --state-interval 500
```

### 🧱 Snapshot from Checkpoint Files

If you already have checkpoint `.bin` files from a previous run, you can render them with custom settings:

```bash
# Generate snapshots from existing checkpoints
make snapshot DIR=outputs/my_run OUT=snap1

# With custom camera and bloom settings
make snapshot DIR=outputs/my_run OUT=snap_iso CAMERA=isometric BLOOM_RED=2.5 BLOOM_BLUE=0.8

# With camera angle (degrees from vertical)
make snapshot DIR=outputs/my_run OUT=snap_rotated CAMERA_ANGLE=45

# Single run (no polling loop)
make snapshot DIR=outputs/my_run OUT=snap_final SINGLE=1
```

#### Create Video from Snapshots

```bash
# Create video from snapshots (24 fps)
make movie DIR=outputs/my_run/snapshots

# With step numbers overlay
make movie DIR=outputs/my_run/snapshots STAMP=1

# Slower video (12 fps)
make movie DIR=outputs/my_run/snapshots FPS=12
```

Makefile variables:

| Variable | Description | Default |
|----------|-------------|---------|
| `DIR` | Run name (outputs/DIR/) | (required) |
| `OUT` | Snapshot subfolder name | (required for snapshot) |
| `CAMERA` | Camera view: `top` or `isometric` | `top` |
| `BLOOM_RED` | Red bloom intensity (0.0-4.0) | (from config) |
| `BLOOM_BLUE` | Blue bloom intensity (0.0-4.0) | (from config) |
| `CAMERA_ANGLE` | Camera angle in degrees (0-360) | - |
| `WIDTH` | Snapshot width in pixels | `3840` |
| `HEIGHT` | Snapshot height in pixels | `2160` |
| `HEADLESS` | Use xvfb for headless rendering (0/1) | `1` |
| `DELAY` | Delay between renders in seconds | - |
| `SINGLE` | Run once instead of polling loop | - |
| `STAMP` | Add step numbers overlay (0/1) | `0` |
| `FPS` | Video framerate | `24` |

Or use the script directly:

```bash
bash snapshot_from_folder.sh \
  --state-glob "outputs/my_run/states/physics_state_step_*.bin" \
  --snapshot-dir "outputs/my_run/snap_iso" \
  --config outputs/my_run/simulation.cfg \
  --camera isometric \
  --bloom-red 2.5 \
  --bloom-blue 0.8 \
  --headless \
  --single
```

Argument reference (worker tools):

* `checkpoint_snapshot_worker.sh`:
  * `--state-glob <glob>`: checkpoint file pattern to watch (default: `outputs/physics_state_step_*.bin`)
  * `--snapshot-dir <dir>`: PNG output dir (default: `outputs/checkpoint_snapshots`)
  * `--camera <value>`: `top` or `isometric` (default: `top`)
  * `--poll-interval <seconds>`: scan cadence (default: `2`)
  * `--headless`: run renderer through `xvfb-run`
* `run_physics_with_worker.sh`:
  * worker args: `--worker-camera`, `--worker-snapshot-dir`, `--worker-poll-interval`, `--worker-headless`
  * physics args are passed after `--`
  * required physics args: `--state-out <path>` and `--state-interval <N>` (`N > 0`)

### 5) Replay From Saved State

Preview one saved state:

```bash
bash run_sim.sh --state-in outputs/physics_state.bin
```

Export snapshots from a saved state without extra physics steps:

```bash
./build/Galaxy_simulation --state-in outputs/physics_state.bin --batch-steps 1 --snapshots 4 --output-dir outputs/from_state
```

`--state-in` cannot be combined with `--physics-only` or `--batch-config`.

### 6) State File Format (`--state-out`)

Header:

* `magic` (`uint32`) = `0x47414C58` (`GALX`)
* `version` (`uint32`) = `1`
* `nb_stars` (`uint32`)

Payload order:

* `positions[nb_stars]` as `dim::Vector4`
* `speeds[nb_stars]` as `dim::Vector4`
* `star_types[nb_stars]` as `int`

## ⚙️ Janus + Performance Update

This branch includes:

* Janus star typing with explicit matter distributions:
  * `Core + halo` (positive center, negative outer region)
  * `Random mix` (positive ratio slider)
  * `Split on X` (two spatial halves)
* `Core + halo` negative density control:
  * `Extra negative density inside core` in `[0, 1]`
  * `0`: core fully positive
  * `1`: core fully negative
  * in `Core + halo` mode: halo stars are always negative
* Janus force multipliers:
  * `Negative Attraction Constant`
  * `Repulsion Constant`
* Color convention:
  * Positive/regular mass: **blue**
  * Negative mass: **red**
* Performance optimizations:
  * Reuse OpenCL command queue
  * Reuse scalar OpenCL buffers and update them in-place
  * Remove unnecessary per-frame acceleration readback
  * Avoid per-frame OpenGL VBO reallocation
  * Upload star type data only at initialization/restart

## 🧪 Batch Experiment Workflow

Use batch mode when you want to simulate many frames and inspect only final snapshots.

### Quick Commands

Single run:

```bash
./build/Galaxy_simulation --batch-steps 10000 --snapshots 6 --output-dir outputs/run_001
```

Run many jobs from config file:

```bash
./build/Galaxy_simulation --batch-config batch_configs/example.batchcfg
```

Use root default file:

```bash
./build/Galaxy_simulation --batch-config batch_jobs.cfg
```

### CLI Argument Reference (`run_batch.sh` / `Galaxy_simulation`)

* `--batch-steps <int>`: number of compute steps before snapshot export (`> 0` enables batch mode).
* `--snapshots <int>`: number of output camera angles (`>= 1`).
* `--output-dir <path>`: destination folder for PNGs.
* `--batch-config <path>`: run all jobs from a config file.
* `--config <path>`: preload one config line before run.
* `--state-in <path>`: load an existing `.bin` state instead of computing.
* `--physics-only`: no renderer path (compute only).

Compatibility notes:

* `--state-in` cannot be combined with `--physics-only` or `--batch-config`.
* `run_batch.sh` supports:
  * positional form: `bash run_batch.sh [steps] [snapshots] [output_dir]`
  * config form: `bash run_batch.sh --batch-config <path> [steps] [snapshots] [output_dir]`
* if root `batch_jobs.cfg` exists, `bash run_batch.sh` uses it automatically.

### Batch Config File Format

* each non-comment line is one job
* each line is `key=value` tokens
* lines can start with `SIMCFG`
* recommended per-line keys:
  * simulation: `simulation_type step smoothing_length interaction_rate nb_stars galaxy_diameter galaxy_thickness galaxies_distance stars_speed black_hole_mass negative_attraction_constant repulsion_constant matter_distribution type_diameter positive_ratio core_extra_negative_density camera_view`
  * batch: `batch_steps snapshots output_dir`

Accepted enum values:

* `simulation_type`: `Galaxy`, `Collision`, `Universe` (also accepted: `0`, `1`, `2`)
* `matter_distribution`: `CoreHalo`, `RandomMix`, `SplitX` (also accepted: `0`, `1`, `2`; `Core+halo` also accepted)
* `camera_view`: `isometric`, `top` (also accepted: `iso`, `0`, `1`)

Minimal line template:

```text
SIMCFG simulation_type=Galaxy nb_stars=20000 batch_steps=2000 snapshots=4 output_dir=outputs/run_001
```

Full line template:

```text
SIMCFG simulation_type=Galaxy step=0.001000 smoothing_length=1.000000 interaction_rate=0.050000 nb_stars=20000 galaxy_diameter=100.000000 galaxy_thickness=5.000000 galaxies_distance=75.000000 stars_speed=20.000000 black_hole_mass=1000.000000 negative_attraction_constant=1.000000 repulsion_constant=1.000000 matter_distribution=CoreHalo type_diameter=40.000000 positive_ratio=0.500000 core_extra_negative_density=0.000000 camera_view=top batch_steps=2000 snapshots=4 output_dir=outputs/run_001
```

Recommended workflow:

1. Start interactive mode (`./build/Galaxy_simulation`) and tune settings until the setup is close to your target physics.
2. Click `Restart` after changing any setting under `Applies after restart`.
3. In the menu, use `Batch config export`:
   * set output file path (default: `batch_configs/user.batchcfg`)
   * click `Append current SIMCFG to file`
4. Repeat for each setup you want to queue.
5. Run the whole file with:
   ```bash
   bash run_batch.sh --batch-config batch_configs/user.batchcfg
   ```
6. Compare the exported snapshots across runs.

Choosing initial parameters:

* `Core + halo`:
  * `Type Diameter` defines the positive core sphere.
  * outside that sphere is halo and always negative.
  * `Extra negative density inside core` injects negative stars into the core (`0` to `1`).
  * start around `type_diameter = 0.4 * galaxy_diameter` and extra density `0.1`, then sweep.
* `Random mix`:
  * use `Positive Ratio` (for example `0.2`, `0.5`, `0.8`) to test segregation behavior.
* `Split on X`:
  * good for interface-instability tests.
* For reduced-particle experiments:
  * decrease `nb_stars`
  * compensate dynamics with `Negative Attraction Constant` and `Repulsion Constant`
  * keep `step` and `smoothing_length` stable while you tune force multipliers.

## ✅ Validate Optimized Runs

Before launching cloud runs, validate local behavior:

1. Build and run:
   ```bash
   bash unix_run.sh
   ./build/Galaxy_simulation
   ```
2. Confirm expected visuals:
   * positive stars are blue
   * negative stars are red
3. Confirm initialization counts in terminal logs:
   * `[Init] stars+=... stars-=... core=... halo=...`
   * this lets you verify the real positive/negative assignment for each restart
   * a copy/paste one-line config is printed as:
     `SIMCFG key=value ...`
4. Confirm batch export:
   ```bash
   ./build/Galaxy_simulation --batch-steps 2000 --snapshots 4 --output-dir outputs/check
   ```
5. Compare timing at same parameters before/after optimization.

Notes:

* Batch mode still needs an OpenGL/X11 context on Linux.
* For server VMs without display, use a virtual display (for example `xvfb`) or add a future true headless mode.

## ☁️ Cloud VM

For remote runs when your local machine is limited:

1. Setup VM dependencies + build:
   * `bash cloud/setup_ubuntu_vm.sh`
2. Physics-only run on VM (no renderer loop):
   * `bash run_physics.sh --device cpu --config simulation.cfg --steps 10000 --state-out outputs/vm_state.bin`
3. Transfer output back locally:
   * `scp ubuntu@<vm-ip>:~/Galaxy_simulation/outputs/vm_state.bin ./outputs/`
4. Existing remote-render options are still available:
   * interactive mode over VNC: `VNC_PASSWORD='change_me' bash cloud/start_live_vnc.sh`
   * headless batch snapshot export via Xvfb: `bash cloud/run_headless_batch.sh --batch-config batch_configs/example.batchcfg`
   * queued batch files: `bash cloud/process_batch_queue.sh`

Full guide: [`cloud/README.md`](cloud/README.md)

## 📝 Changes

Detailed change log: see [`CHANGE.md`](CHANGE.md).

## 🧩 Troubleshooting

* `Could not find SFML`: make sure submodules are present (`git submodule update --init --recursive`).
* `Could NOT find OpenCL`: use `bash unix_run.sh` (it passes the required OpenCL paths).
* `No platforms found!`: OpenCL loader is present but no OpenCL driver ICD is installed.
  On Arch Linux:
  `sudo pacman -S --needed pocl` (CPU fallback) or install the NVIDIA-matching OpenCL package (for example `opencl-nvidia-580xx` for 580xx drivers).
  Verify with `clinfo`: `Number of platforms` must be at least `1`.
* `Error: failed to build compute program on all devices` / `Build Status: -2`:
  * OpenCL platform exists, but kernel compilation failed on available devices.
  * verify with `clinfo` that the intended device is visible
  * on Linux, install/update the vendor OpenCL runtime (NVIDIA/AMD/Intel) or use `pocl` as fallback
  * keep a CPU OpenCL ICD installed on cloud VMs without GPU
* `run_physics.sh` finished but no file was created:
  * expected if `--state-out` was not provided.
  * example: `bash run_physics.sh --steps 2000 --state-out outputs/physics_state.bin`
* `Failed to open X11 display`: run from a desktop session with a valid `DISPLAY` variable.

<br/>

# 🧪 Simulation variants

* Add different star types at random in proportion.
* Add different star types from a diameter threshold (current modification).
* Select matter distribution in-app:
  * `Core + halo`: positive core, negative halo
  * `Random mix`: random positive/negative based on ratio
  * `Split on X`: one half positive, one half negative

<br/>

# 🗓️ Releases

If you just want to test the program without editing the source code, go see the [**Releases**](https://github.com/angeluriot/Galaxy_simulation/releases).

<br/>

# 🧪 Tests

### A unique galaxy

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_5.png" width="500">
</p>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_6.png" width="500">
</p>

<br/>

### Mixing 2 different types of stars

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_7.png" width="500">
</p>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_8.png" width="500">
</p>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_9.png" width="500">
</p>

<br/>

### Two galaxies colliding

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_10.png" width="500">
</p>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_11.png" width="500">
</p>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_12.png" width="500">
</p>

<br/>

### Universe simulations

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_14.png" width="500">
</p>

<p align="center">
	<img src="https://raw.githubusercontent.com/angeluriot/Galaxy_simulation/master/resources/misc/galaxy_13.png" width="500">
</p>

<br/>

# 🙏 Credits

* [**Angel Uriot**](https://github.com/angeluriot) : Creator of the project.

<br>

---

# 📊 Galaxy Physics Documentation

This section documents how the physics is calculated in the galaxy simulation and how negative mass is associated with particles in different initial conditions.

## Overview

The simulation uses an **N-body gravitational simulation** with two types of matter: **positive mass** (attractive) and **negative mass** ( repulsive when interacting with positive matter). The computation runs entirely on the GPU via OpenCL.

## Force Calculation

### N-Body Interaction (GPU Kernel)

The force on each particle is computed in the OpenCL kernel `cl_compute_shader.cl` using a brute-force N² approach:

```c
for each particle i:
    for each particle j (sampled by interaction_rate):
        if i != j:
            vector = position[j] - position[i]
            if types[i] == types[j]:
                // Same sign: attraction
                sign = (type == -1) ? negative_attraction_constant : 1.0
            else:
                // Different signs: repulsion
                sign = -repulsion_constant
            
            acceleration += sign * normalize(vector) / (|vector|² + smoothing_length)
```

**Key parameters:**

| Parameter | Description |
|-----------|-------------|
| `interaction_rate` | Fraction of particle pairs sampled per step (0-1). Higher = more accuracy, more compute. |
| `smoothing_length` | Softening parameter to avoid singularities at close distances |
| `negative_attraction_constant` | Multiplier for negative-negative attraction (default: 1.0) |
| `repulsion_constant` | Multiplier for positive-negative repulsion (default: 1.0) |

### Central Black Hole

A central mass at the origin exerts additional gravitational attraction:

```
acceleration += black_hole_mass * normalize(-position) / (|position|² + smoothing_length)
```

### Integration (Euler Method)

Velocity and position are updated each timestep:

```
velocity += step * acceleration
position += step * velocity
```

## Initial Matter Distributions

When initializing a galaxy, each star is assigned a **type** (+1 for positive mass, -1 for negative mass). The distribution depends on the selected `matter_distribution` mode:

### 1. Core + Halo (Default)

```
core_radius = type_diameter / 2

for each star:
    if distance_from_center <= core_radius:
        // Inside core
        if random() <= core_extra_negative_density:
            type = -1  // Negative
        else:
            type = +1  // Positive
    else:
        // Outside core (halo)
        type = -1  // Always negative
```

- **Positive core**: Stars within `type_diameter` sphere
- **Negative halo**: Stars outside the core radius
- **Extra negative density**: The `core_extra_negative_density` parameter (0-1) injects negative stars into the core region, allowing hybrid configurations

### 2. Random Mix

```
for each star:
    if random() <= positive_ratio:
        type = +1
    else:
        type = -1
```

Stars are randomly assigned positive/negative based on the `positive_ratio` parameter (e.g., 0.5 = 50% each).

### 3. Split on X

```
for each star:
    if position.x <= 0:
        type = +1  // Left half
    else:
        type = -1  // Right half
```

The simulation volume is divided along the X-axis, creating a positive half-space and a negative half-space.

## Force Sign Conventions

| Interaction | Force Direction |
|-------------|-----------------|
| Positive + Positive | Attraction (gravity) |
| Negative + Negative | Attraction (scaled by `negative_attraction_constant`) |
| Positive + Negative | Repulsion (scaled by `repulsion_constant`) |

This creates a system where:
- Positive mass clusters under mutual gravitation
- Negative mass also clusters (self-attraction)
- Positive and negative matter repel each other

## Color Convention

- **Blue stars**: Positive (regular) mass
- **Red stars**: Negative mass

## Initial Velocity Setup

For **Galaxy** simulation type:

```
tangential_speed = initial_speed_for_star(config, star_type)
velocity = normalize(position × up_vector) * tangential_speed
```

Stars receive an initial tangential velocity perpendicular to the radial direction, creating orbital motion around the galactic center.

For **Universe** simulation type:

```
radial_speed = (distance / galaxy_radius) * initial_speed
velocity = normalize(position) * radial_speed
```

Stars receive an outward radial velocity proportional to their distance from center, simulating cosmic expansion.

---

*This documentation applies to the Galaxy simulation variant. Collision and Universe simulations use different initial velocity configurations but the same underlying force calculation.*
