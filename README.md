# Building on Linux

```
./build/Galaxy_simulation 
```
to run the built

```
bash unix_run.sh
```
to built

error 0: install libglew-dev

error 1 : https://stackoverflow.com/questions/56858213/how-to-create-nvidia-opencl-project/57017982#57017982

https://github.com/ProjectPhysX/OpenCL-Wrapper/tree/master/src/OpenCL and put the file in the incldes with good folders



error 2: that was useful
```
sudo apt-get install freeglut3-dev
```
from https://askubuntu.com/questions/96087/how-to-install-opengl-glut-libraries

## Two differents simulation modifications 
 
- Add different stars type at random in proportion
- Add different stars type from a certain diameter (The current modification) 


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
* **[🛠️ Install](#%EF%B8%8F-install)**
	* [🦴 Skeleton project install](#-skeleton-project-install)
	* [📦 Additionnal dependencies](#-additionnal-dependencies)
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

# 🛠️ Install

## 🦴 Skeleton project install

This repository is based on my [**Graphics skeleton project**](https://github.com/angeluriot/Graphics_skeleton), so go see its [**install**](https://github.com/angeluriot/Graphics_skeleton#install) part first.

<br/>

## 📦 Additionnal dependencies

Then, you will also need the [**OpenCL**](https://www.khronos.org/opencl/) library.

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
