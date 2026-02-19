# Architecture Guide (Beginner Friendly)

This file explains, in simple terms, how the project is now organized and how to use it with a cloud VM.

## 1) Big picture

You now have **two programs**:

1. `Galaxy_simulation`
   - This is the normal app with graphics (OpenGL window).
   - Use this on your local machine to see stars.

2. `Galaxy_physics`
   - This runs only the physics calculations.
   - It can run on a cloud VM and save a result file.

So the idea is:

1. Run physics in the cloud.
2. Save output to a file.
3. Download that file to your computer.
4. (Later step) load/show it locally in renderer tools.

## 2) Simple words

- VM: a remote Linux computer you rent/use online.
- OpenCL: computes physics on CPU/GPU.
- OpenGL: draws graphics.
- Headless: runs without opening a graphics window.
- `state file`: binary file with star positions/speeds/types.

## 3) What is already done

- Physics code and render code are separated in architecture.
- `Galaxy_physics` binary exists and runs physics loop.
- It can save a file with final simulation state (`--state-out`).

## 4) What is NOT done yet

- No live network stream yet from VM -> local renderer.
- Current remote flow is file-based (run, save, copy file).

## 5) First cloud VM setup (copy/paste)

Example assumes Ubuntu VM.

### On your VM

```bash
git clone --recurse-submodules <your-repo-url>
cd Galaxy_simulation
bash cloud/setup_ubuntu_vm.sh
```

### Build (if needed)

```bash
bash unix_run.sh
```

### Run physics-only in VM

```bash
bash run_physics.sh --config simulation.cfg --steps 10000 --state-out outputs/vm_state.bin
```

If you use a batch-style config line file, you can also point `--config` to that file.

## 6) Download result back to your computer

From your local machine:

```bash
scp ubuntu@<vm-ip>:~/Galaxy_simulation/outputs/vm_state.bin ./outputs/
```

Now you have the VM result file locally in `./outputs/vm_state.bin`.

## 7) Local renderer today

Today, local visualization is still through the normal app:

```bash
bash run_sim.sh
```

or batch snapshots:

```bash
bash run_batch.sh 200 2 outputs/local_test
```

The direct loader for `vm_state.bin` into renderer is the next step we can add.

## 8) If something fails

### "No platforms found!"

OpenCL runtime is missing.

### "failed to build compute program on all devices"

OpenCL exists but kernel compile failed on available device.
Usually fixed by installing/updating OpenCL runtime (or using another device/runtime).

### "Failed to open X11 display"

Graphics app was started in environment with no display.
Use `Galaxy_physics` for cloud/headless compute jobs.

## 9) Quick command cheat sheet

Build:

```bash
bash unix_run.sh
```

Run local graphics app:

```bash
bash run_sim.sh
```

Run physics-only:

```bash
bash run_physics.sh --steps 2000
```

Run physics-only with config + output file:

```bash
bash run_physics.sh --config simulation.cfg --steps 10000 --state-out outputs/vm_state.bin
```
