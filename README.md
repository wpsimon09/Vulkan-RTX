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

Shaders are compiled using t--no-skip-unchangedhe `slangc` compiler, which can be downloaded from [here](https://github.com/shader-slang/slang/releases).

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
python compileSlang.py --verbose --no-skip-unchanged
```

The `--no-skip-unchanged` option forces all shaders to compile, even if they haven’t changed. '--verbose' flag will print detailed messages about compilation process

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
- Ray traced AO
- Bilateral filter denoiser

## Little showcase


https://github.com/user-attachments/assets/550c47df-4557-45c6-b38a-dce62449e8c7

<img width="2868" height="1722" alt="image" src="https://github.com/user-attachments/assets/6fe78e8d-380b-430f-bb3e-dc0127fa6422" />

![image](https://github.com/user-attachments/assets/69a1ac89-3466-49b0-b9ef-e5a453318cb1 )

<img width="2869" height="1545" alt="image" src="https://github.com/user-attachments/assets/cbb1b019-7392-47c3-8a4f-a992c9831ddd" />

<img width="2880" height="1620" alt="image" src="https://github.com/user-attachments/assets/66cfda6b-8572-41db-8da5-efa4a3ff78ec" />

<img width="2871" height="1545" alt="Screenshot From 2025-07-25 20-49-26" src="https://github.com/user-attachments/assets/fa6218d8-9176-4c48-8754-d363c81c0fa8" />

