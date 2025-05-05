# Vulkan RTX

Simple Vulkan physically based render that can render meshes in classic rasterisation pipeline, but also contains Path tracer that uses `RayTracingPipeline` to utilise RT cores for a real time path tracing.

## Used technology

- _Rendering API_ - Vulkan (`vulkan.hpp`)
- _Shading Language_ - Slang
- _Language_ - C++
- _Math_ - glm  
- _Dear::ImGui_ - User interface

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
mkdir cache ## create cache directory, TODO: this will be part of the build script
cd build ## go to the build directory
cmake .. ## write build files
cmake --build .
```
---

## Little showcase

### Editor

![image](https://github.com/user-attachments/assets/69a1ac89-3466-49b0-b9ef-e5a453318cb1)


![image](https://github.com/user-attachments/assets/d00f9703-44d1-4e87-aad9-2bf362cd7e9e)


## Early version of path tracing 

### RTX OFF
![image](https://github.com/user-attachments/assets/7a3af6aa-d296-49ac-9d4e-4aede9a9cb06)


### RTX ON
![image](https://github.com/user-attachments/assets/7b97e6ca-bb6f-402c-ade9-394566150c87)


