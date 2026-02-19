# VM Checklist (Command-Only)

Use this as a strict recipe.

## A) Do you need a GPU in cloud?

Short answer: **No, not required**.

- `Galaxy_physics` uses OpenCL.
- It can run on CPU (for example with `pocl`).
- GPU is optional for speed.

For first tests, CPU is fine.

---

## B) Recommended path (persistent VM)

### 1) Create Ubuntu VM

- Ubuntu 22.04 or 24.04
- 2 to 4 vCPUs minimum
- 8 GB RAM if possible

### 2) SSH into VM

```bash
ssh ubuntu@<vm-ip>
```

### 3) Clone project

```bash
git clone --recurse-submodules <your-repo-url>
cd Galaxy_simulation
```

### 4) Install dependencies + build

```bash
bash cloud/setup_ubuntu_vm.sh
```

### 5) Run physics-only

```bash
bash run_physics.sh --device cpu --config simulation.cfg --steps 10000 --state-out outputs/vm_state.bin
```

Notes:

- add `--state-out ...` if you want a file output
- without `--state-out`, run completes with no saved file
- progress is printed periodically during the loop

### 6) Copy result back to local machine

Run this on your local machine (not on VM):

```bash
scp ubuntu@<vm-ip>:~/Galaxy_simulation/outputs/vm_state.bin ./outputs/
```

---

## C) Google Colab fallback (no credit card)

This is for temporary testing only.

### 1) Start notebook

- Runtime: CPU (recommended first)

### 2) In a notebook cell, install tools

```bash
!apt-get update -y
!apt-get install -y git build-essential cmake libglew-dev freeglut3-dev ocl-icd-opencl-dev pocl-opencl-icd
```

### 3) Clone + build

```bash
!git clone --recurse-submodules <your-repo-url>
%cd Galaxy_simulation
!bash unix_run.sh
```

### 4) Run physics-only

```bash
!bash run_physics.sh --device cpu --config simulation.cfg --steps 2000 --state-out outputs/colab_state.bin
```

### 5) Download result

```python
from google.colab import files
files.download('outputs/colab_state.bin')
```

Notes:

- Colab is not persistent (sessions reset).
- OpenCL support can vary by runtime.
- It is good for quick proof-of-concept, not stable production workflow.

---

## D) If command fails

### `No platforms found!`

OpenCL runtime missing.

Install:

```bash
sudo apt-get install -y ocl-icd-opencl-dev pocl-opencl-icd
```

### `failed to build compute program on all devices`

OpenCL exists but kernel compile failed on available device.

Try:

1. Use another machine/runtime.
2. Verify available devices:
   ```bash
   clinfo | head -n 50
   ```
3. Keep CPU fallback installed (`pocl-opencl-icd`).
