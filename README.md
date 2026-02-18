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
* **[🐧 Build on Linux](#-build-on-linux)**
* **[🛠️ Install](#%EF%B8%8F-install)**
	* [📦 Dependencies](#-dependencies)
	* [▶️ Build](#%EF%B8%8F-build)
	* [🚀 Run](#-run)
	* [🧩 Troubleshooting](#-troubleshooting)
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

# 🐧 Build on Linux

```bash
bash unix_run.sh
```

This script:

* initializes/upgrades git submodules (`--init --recursive`)
* configures CMake with `Unix Makefiles`
* uses the OpenCL include/library shipped in this repository
* builds the executable in `./build`

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

## 🚀 Run

```bash
./build/Galaxy_simulation
```

## 🧩 Troubleshooting

* `Could not find SFML`: make sure submodules are present (`git submodule update --init --recursive`).
* `Could NOT find OpenCL`: use `bash unix_run.sh` (it passes the required OpenCL paths).
* `No platforms found!`: OpenCL loader is present but no OpenCL driver ICD is installed.
  On Arch Linux:
  `sudo pacman -S --needed pocl` (CPU fallback) or install the NVIDIA-matching OpenCL package (for example `opencl-nvidia-580xx` for 580xx drivers).
  Verify with `clinfo`: `Number of platforms` must be at least `1`.
* `Failed to open X11 display`: run from a desktop session with a valid `DISPLAY` variable.

<br/>

# 🧪 Simulation variants

* Add different star types at random in proportion.
* Add different star types from a diameter threshold (current modification).

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
