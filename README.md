# Vulkan RTX

Simple Vulkan physically based render that can render meshes in classic rasterisation pipeline, but also contains Path tracer that uses `RayTracingPipeline` to utilise RT cores for a real time path tracing.

## Used technology

- _Rendering API_ - Vulkan (`vulkan.hpp`)
- _Shading Language_ - Slang
- _Language_ - C++
- _Math_ - glm  
- _Dear::ImGui_ - User interface

## Build

To run this project, for now it is crucial that your GPU supports `VK_KHR_ray_tracing_pipeline` ! 

### 1. Clone this repo

```sh
git clone https://github.com/wpsimon09/Vulkan-RTX.git --recursive  ## recursive has to be there 
```

### 2. Download GLFW

- this library is used for window creation and provides context for Vulkan

**On windows**
- [downald link](https://www.glfw.org/)

**On Linux**
     
```sh
# On Ubuntu/Debian:
sudo apt update && sudo apt install -y libglfw3 libglfw3-dev

# On Arch Linux:
sudo pacman -S glfw --noconfirm

# On Fedora (Red Hat-based distros):
sudo dnf install -y glfw glfw-devel

# On openSUSE:
sudo zypper install glfw-devel

```
### 3. Download GLM

**On windows**
- [downald link](https://sourceforge.net/projects/glm.mirror/)

**On Linux**
```bash
# On Ubuntu/Debian:
sudo apt update && sudo apt install -y libglm-dev

# On Arch Linux:
sudo pacman -S glm --noconfirm

# On Fedora (Red Hat-based distros):
sudo dnf install -y glm-devel

# On openSUSE:
sudo zypper install glm-devel

```

### 4. Build the project

```sh
sh build.sh ## default is debug

## in debug mode (explicitly)
sh build.sh debug

## in release mode (app wont work in release mode now)
sh build.sh release
```
---

## Compiling shaders

Note that you must have python installed verify this by running 

```
python --help
```

Shaders are compiled using the `slangc` compiler, which can be downloaded from [here](https://github.com/shader-slang/slang/releases).

> Shaders are already precompiled, so you don't need to do this. Only recompile them when you change the shaders.

To get started, navigate to the `Shaders` directory:

```bash
cd Shaders
```
Then, open `compileSlang.py` and change the variable `SLANGC_PATH` to point to the `slangc` executable.

*Example:*  
`SLANGC_PATH = "/home/user/SDKs/slang/bin/slangc"`

Once done, use Python to compile the shaders from the `Shaders` directory:
```bash
## assuming you are in the Shaders directory
python compileSlang.py --verbose
```
 


## Little showcase

### Editor

![image](https://github.com/user-attachments/assets/69a1ac89-3466-49b0-b9ef-e5a453318cb1)


![image](https://github.com/user-attachments/assets/d00f9703-44d1-4e87-aad9-2bf362cd7e9e)

![image](https://github.com/user-attachments/assets/2d217da5-5d04-482b-b938-9b436d28d078)

## Early version of path tracing 

### RTX OFF
![image](https://github.com/user-attachments/assets/5ce4636b-7d55-448c-9a10-c18c1e0944d1)


### RTX ON
![image](https://github.com/user-attachments/assets/3f7a2fc2-b993-499d-bbe8-eb2047063820)


