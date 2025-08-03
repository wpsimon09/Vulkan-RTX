#ifndef RENDERINGUTILS_HPP
#define RENDERINGUTILS_HPP

#include "vulkan/vulkan.hpp"

namespace Renderer {
void ConfigureViewPort(const vk::CommandBuffer& cmdB, float w, float h);
}


#endif