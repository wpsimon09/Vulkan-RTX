//
// Created by wpsimon09 on 20/12/24.
//

#ifndef IMGUIINITIALIZER_HPP
#define IMGUIINITIALIZER_HPP

#include <GLFW/glfw3.h>

#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

struct ImGuiIO;

namespace VulkanCore
{
    class VRenderPass;
    class VulkanInstance;
    class VDevice;
}

namespace VulkanUtils {

class ImGuiInitializer {
public:
    explicit ImGuiInitializer(
        const VulkanCore::VDevice& device,
        const VulkanCore::VulkanInstance& instance,
        const VulkanCore::VRenderPass& renderPass,
        const WindowManager& windowManager
    );
    void Initialize();
    void Render(VulkanCore::VCommandBuffer& commandBuffer);
private:

    const VulkanCore::VDevice& m_device;
    const VulkanCore::VulkanInstance& m_instance;
    const VulkanCore::VRenderPass& m_renderPass;
    const WindowManager& m_windowManager;

    vk::PipelineCache m_imguiPipelineCache;
    vk::DescriptorPool m_imguiDescriptorPool;

    ImGuiIO* m_io;
};

} // VulkanUtils

#endif //IMGUIINITIALIZER_HPP
