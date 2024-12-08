# Vulkan RTX

Simple Vulkan physically based render that can render meshes in classic rasterisation pipeline. But also contains Ray tracer that renders the scene on the full screen Quad

The goal is to eventually transition from a purely shader-based ray tracing implementation to leveraging Vulkan's dedicated ray tracing pipeline

## Used technology

_Rendering API_ - Vulkan (`vulkan.hpp`)
_Shading Language_ - Slang
_Language_ - C++

## Notes

- Current code needs quite a lot of refactoring and simplification.
- Ray Tracer **does not** represent the scene that is rendered with raster pipeline
- Ray Tracer **does not** yet use accumulated average to de-noise the rendered image
- This is quite silly project for me to learn how ray tracing and Vulkan works

## Little showcase 

