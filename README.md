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
- Ray Tracer **does not** represent the scene that is rendered with raster pipeline
- Ray Tracer **does not** yet use accumulated average to de-noise the rendered image
- This is quite silly project for me to learn how ray tracing and Vulkan works

## Little showcase

### Editor
![Screenshot From 2025-03-01 15-36-51](https://github.com/user-attachments/assets/3b28c2e6-ecd4-457e-818f-900dd92dffb3)

![image](https://github.com/user-attachments/assets/d3b2c80b-2ad4-4eef-96e0-6b3747bfcaf9)

### RTX OFF
![image](https://github.com/user-attachments/assets/217f5722-8050-405f-aa7c-cb540129c5a7)


### RTX ON
![image](https://github.com/user-attachments/assets/82947f57-1f2e-4615-96e6-388133e085ae)


