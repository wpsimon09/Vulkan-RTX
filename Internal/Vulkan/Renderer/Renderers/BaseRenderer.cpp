//
// Created by wpsimon09 on 23/12/24.
//

#include "BaseRenderer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


namespace Renderer {
BaseRenderer::BaseRenderer(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_commandBuffers.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
}

void BaseRenderer::Destroy()
{

    m_renderTargets->Destroy();
}
}  // namespace Renderer