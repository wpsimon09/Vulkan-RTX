# Vulkan RTX

Simple Vulkan physically based render that can render meshes in classic rasterisation pipeline, but also contains Path tracer that uses `RayTracingPipeline` to utilise RT cores for a real time path tracing.

## Used technology

- _Rendering API_ - [Vulkan](https://www.vulkan.org/) (`vulkan.hpp`)
- _Shading Language_ - [Slang](https://shader-slang.org/)
- _Language_ - C++
- _Math_ - [glm](https://github.com/g-truc/glm)  
- _User interface_ - [Dear ImGui](https://github.com/ocornut/imgui)
- _.glTF parsing_ - [fastgltf](https://github.com/spnda/fastgltf)
- _.ini parsing_ - [mINI](https://github.com/metayeti/mINI)
- _SPIR-V Reflection_ - [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)
- _Gizmo_ - [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)

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

## Features:
- Model Loading  
- Scene Exporting (To glTF)  
- Forward Renderer  
- Area Light With LTC  
- Ray Traced Pixel Perfect Shadows  
- Real time path tracing (with RT cores)
- Image Based Lighting  
- ACES Tone Mapping  
- Scene Editor With Gizmos  
- Real Time Material Customisation
- Multy threaded lazy texture loading
- Bindless descritptors for material textures
- Large scale homogeneous participating media rendering

## Little showcase

![image](https://github.com/user-attachments/assets/69a1ac89-3466-49b0-b9ef-e5a453318cb1 )

![Screenshot From 2025-06-03 10-31-22](https://github.com/user-attachments/assets/6d313d62-2d93-40f8-beb5-18aadbd71e1f )

![image](https://github.com/user-attachments/assets/2d217da5-5d04-482b-b938-9b436d28d078 )

![image](https://github.com/user-attachments/assets/2170c8a6-998f-45f5-91d7-6fdddd1c271a)

