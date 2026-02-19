# Google Colab Session Guide (Step by Step)

This guide is for your first full run in Google Colab.

## 1) Open Colab

1. Go to `https://colab.research.google.com`
2. Click `File` -> `New notebook`
3. Click `Runtime` -> `Change runtime type`
4. Set runtime to `CPU` (good for first test)

## 2) Run these cells in order

### Cell A: install dependencies

```python
!apt-get update -y
!apt-get install -y git build-essential cmake libglew-dev freeglut3-dev ocl-icd-opencl-dev pocl-opencl-icd clinfo
```

### Cell B: clone repo

```python
!git clone --recurse-submodules https://github.com/cittadhammo/Galaxy_simulation.git
%cd Galaxy_simulation
```

### Cell C: checkout your branch

```python
!git fetch --all
!git checkout refactor/headless-physics-split
!git submodule update --init --recursive
!git branch --show-current
```

Expected output:

`refactor/headless-physics-split`

### Cell D: build

```python
!bash unix_run.sh
```

### Cell E: quick OpenCL check

```python
!clinfo | head -n 80
```

### Cell F: quick physics test

```python
!bash run_physics.sh --steps 200 --state-out outputs/colab_state_small.bin
```

Force CPU or ask for GPU preference:

```python
!bash run_physics.sh --device cpu --steps 200 --state-out outputs/colab_state_small.bin
!bash run_physics.sh --device gpu --steps 200 --state-out outputs/colab_state_small.bin
```

Notes:

- `--device gpu` means "prefer GPU if available".
- If no GPU OpenCL device is available, code falls back to any available device.
- Startup logs print selected OpenCL platform/device.

### Cell G: larger physics run (optional)

```python
!bash run_physics.sh --config simulation.cfg --steps 2000 --state-out outputs/colab_state.bin
```

### Cell H: download output file

```python
from google.colab import files
files.download('outputs/colab_state_small.bin')
# files.download('outputs/colab_state.bin')  # uncomment if you ran Cell G
```

## 3) Common issues

### Error: `No platforms found!`

OpenCL runtime not available in session.

Re-run install cell:

```python
!apt-get install -y ocl-icd-opencl-dev pocl-opencl-icd clinfo
!clinfo | head -n 80
```

### Error: `failed to build compute program on all devices`

OpenCL device exists but kernel compile failed on this Colab runtime.

Try:

1. `Runtime` -> `Restart runtime`
2. Re-run all cells
3. If still failing, Colab runtime is incompatible for this session; retry later or use a VM

### Notebook reset/disconnect

Colab sessions are temporary. Keep output files downloaded right after generation.

## 4) Important note

Colab is for quick tests only.

For stable repeated runs, use a persistent VM (see `docs/VM_CHECKLIST.md`).
