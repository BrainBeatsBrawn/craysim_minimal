# craysim_minimal

The most minimal craysim program I could write.
Opens a simple 3D world (data/natural_env.gltf), with a spherical compound-ray camera (data/eyes/poly.eye).
Allows you to move around on the landscape (w-a-s-d keys, see also Ctrl-h). The spherical eye leaves a little trail of yellow breadcrumbs showing you where it has been.

![Craysim_minimal in action](https://github.com/BrainBeatsBrawn/craysim_minimal/blob/main/data/Compound-ray_sim.png?raw=true)


## Prerequisites

Before compiling craysim_minimal, obtain and compile NVidia Optix 8.0 and obtain,
compile and `make install` compound-ray from:

https://github.com/BrainBeatsBrawn/compound-ray

You will need to use gcc-12 or gcc-11 to compile compound-ray (it may be possible with a version of clang, too, but I've not verified this).

Install the mathplot dependencies (wrt branch dev/modules):
```bash
sudo apt install build-essential cmake git ninja-build \
                 librapidxml-dev freeglut3-dev libglu1-mesa-dev libxmu-dev \
                 libxi-dev libglfw3-dev libfreetype-dev libhdf5-dev
```

To compile craysim_minimal, you need to use clang-20 or higher.

You also need cmake version 3.28.5 or higher. Either `apt install cmake`
on Ubuntu 25+ or download and build cmake from the cmake.org download
page (it's an easy, reliable compile).

## Submodules

This program uses the submodules [sebsjames/mathplot](https://github.com/sebsjames/mathplot), [sebsjames/maths](https://github.com/sebsjames/maths), [nlohmann/json](https://github.com/nlohmann/json), [BrainBeatsBrawn/craysim](https://github.com/BrainBeatsBrawn/craysim), [BrainBeatsBrawn/oces_viewer](https://github.com/BrainBeatsBrawn/oces_viewer) and [tinygltf](https://github.com/sebsjames/tinygltf). The program links to Seb's fork of [compound-ray](https://github.com/BrainBeatsBrawn/compound-ray)

## Build

To compile craysim_minimal, recursively clone it, or if you already cloned, init/update the submodules

```bash
cd craysim_minimal
git submodule init
git submodule update
```

then do a cmake build, passing the cmake variable OptiX_INSTALL_DIR
just as you will have done so when you built compound-ray:

```bash
mkdir build
pushd build
CXX=clang++-20 cmake .. -GNinja -DOptiX_INSTALL_DIR=~/src/NVIDIA-OptiX-SDK-8.0.0-linux64-x86_64
make
popd
```

Now you can run the program

```bash
./build/craysim_minimal -f ./data/natural_env.gltf
```

Author: Seb James
Date: March 2026
