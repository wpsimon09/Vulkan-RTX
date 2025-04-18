# Vulkan RTX

Simple Vulkan physically based render that can render meshes in classic rasterisation pipeline. But also contains Ray tracer that renders the scene on the full screen Quad by utilising the 
ray tracing technology. 

The goal is to eventually transition from a purely shader-based ray tracing implementation to leveraging Vulkan's dedicated ray tracing pipeline


## Used technology

- _Rendering API_ - Vulkan (`vulkan.hpp`)
- _Shading Language_ - Slang
- _Language_ - C++
- _Math_ - glm  
- _Dear::ImGui_ - User interface

## Notes

- Current code needs quite a lot of refactoring and simplification.
- Ray Tracer **does not** yet represent the scene that is rendered with raster pipeline
- Ray Tracer **does not** yet use accumulated average to de-noise the rendered image
- This is quite silly project for me to learn how ray tracing and Vulkan works

## Build

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

### 3. Build the project

```sh
cd Vulkan-RTX ## navigate to repo
mkdir build ## create build directory
cd build ## go to the build directory
cmake -DCMAKE_BUILD_TYPE=Release -S ../ -B . ## write build files
cmake --build .
```
---

## Little showcase

### Editor

![image](https://github.com/user-attachments/assets/69a1ac89-3466-49b0-b9ef-e5a453318cb1)


![image](https://github.com/user-attachments/assets/d00f9703-44d1-4e87-aad9-2bf362cd7e9e)




### RTX OFF
![image](https://github.com/user-attachments/assets/217f5722-8050-405f-aa7c-cb540129c5a7)


### RTX ON
![image](https://github.com/user-attachments/assets/82947f57-1f2e-4615-96e6-388133e085ae)


