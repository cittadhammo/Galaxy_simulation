# 🌌 Galaxy Simulation — Lightning AI Setup Guide

Quick reference for building and running the Galaxy Simulation on Lightning AI Studios, for both CPU and GPU instances.

---

## 1. Clone the Repository

Always clone **with submodules** (SFML and other deps live there):

```bash
git clone --recurse-submodules https://github.com/cittadhammo/Galaxy_simulation.git
cd Galaxy_simulation
```

If you already cloned without submodules, fix it with:

```bash
git submodule update --init --recursive
```

---

## 2. Install Dependencies

Run this on any Lightning AI Studio (Ubuntu):

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  libglew-dev \
  freeglut3-dev \
  ocl-icd-opencl-dev \
  xvfb \
  clinfo \
  ffmpeg \
```

Comment out cloud-incompatible lines (Menu UI panning code and ImageMagick check):

```bash
make cloud-comment
```


---

## 3A. CPU Build

Install a CPU OpenCL driver (POCL):

```bash
sudo apt-get install -y pocl-opencl-icd
```

Verify OpenCL is working:

```bash
clinfo  # should show at least 1 platform
```

Build:

```bash
bash unix_run.sh
```

Run physics on CPU:

```bash
bash run_physics.sh --device cpu --steps 2000 --state-out outputs/result.bin
```

---

## 3B. GPU Build (L40S / A100 / any NVIDIA)

> ⚠️ You must switch to a GPU instance in the Lightning AI dashboard **before** building.

### Check GPU and OpenCL

```bash
nvidia-smi    # may show "Driver/library version mismatch" — this is harmless, ignore it
clinfo        # must show "NVIDIA CUDA" as a platform
```

If `clinfo` shows `Number of platforms: 0`, install the NVIDIA OpenCL ICD:

```bash
sudo apt-get install -y nvidia-opencl-dev opencl-headers ocl-icd-libopencl1
clinfo  # retry — should now show NVIDIA CUDA platform
```

### Build

```bash
rm -rf build      # clean any previous CPU build
bash unix_run.sh
```

### Run physics on GPU

```bash
bash run_physics.sh --device gpu --steps 2000 --state-out outputs/result.bin
```

### GPU tip — push star count way up

On GPU you can go far beyond the CPU limit of ~30,000 stars. Edit `simulation.cfg` and try:

```
nb_stars=200000   # or 500000+ on L40S / A100
```

---

## 4. Verify the Build

```bash
ls build/
# Expected output: Galaxy_simulation  Galaxy_physics
```

---

## 5. Common Workflows

### Run interactive simulation (needs a display)

```bash
./build/Galaxy_simulation
```

### Physics-only (no display needed — best for Lightning AI)

```bash
bash run_physics.sh --device gpu --config simulation.cfg --steps 2000 --state-out outputs/result.bin
```

### Physics-only with NO snapshots (fastest — pure compute)

```bash
bash run_physics.sh --device gpu --steps 2000 --state-out outputs/result.bin
# or via Makefile:
make physic DIR=my_run STEPS=2000 SNAPSHOTS=0
```

### Physics with periodic checkpoints

```bash
bash run_physics.sh --device gpu --steps 5000 \
  --state-out outputs/physics_state.bin \
  --state-interval 500
```

### Headless batch snapshots (uses xvfb virtual display)

```bash
make physic DIR=my_run STEPS=2000 STATE_INTERVAL=40
```

### Generate snapshots from saved checkpoints

```bash
make snapshot DIR=outputs/my_run OUT=snap1
```

### Create a video from snapshots

```bash
make movie DIR=outputs/my_run/snapshots
```

---

## 6. Troubleshooting

| Problem | Fix |
|---|---|
| `Could not find SFML` | Run `git submodule update --init --recursive` |
| `Could NOT find OpenCL` | Use `bash unix_run.sh` (passes OpenCL paths automatically) |
| `No platforms found!` | Install `pocl-opencl-icd` (CPU) or `nvidia-opencl-dev` (GPU) |
| `Failed to initialize NVML` | Harmless — ignore it, OpenCL still works |
| `clinfo` shows 0 platforms on GPU | Install `nvidia-opencl-dev` and retry |
| `Error: ffmpeg not installed` | Run `sudo apt-get install -y ffmpeg` then retry `make movie` |
| `Error: ImageMagick not installed` | Run `sudo apt-get install -y imagemagick` then retry |
| `xvfb-run is not available` | Run `sudo apt-get install -y xvfb` then retry |
| `Failed to open X11 display` | Use `--device cpu/gpu` physics-only mode, or prefix with `xvfb-run` |
| Build succeeded but wrong device used | Delete `build/` and rebuild after switching CPU↔GPU instance |

---

## 7. Key Files

| File | Purpose |
|---|---|
| `unix_run.sh` | Main build script (always use this) |
| `simulation.cfg` | Simulation parameters (edit `nb_stars`, `steps`, etc.) |
| `run_physics.sh` | Physics-only runner (no display needed) |
| `run_physics_with_worker.sh` | Physics + snapshot worker in one command |
| `Makefile` | Shortcuts for common workflows (`make physic`, `make snapshot`, `make movie`) |
| `outputs/` | All results saved here |
