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

namespace VEditor {

class UIContext {
public:
    explicit UIContext(
        const VulkanCore::VDevice& device,
        const VulkanCore::VulkanInstance& instance,
        const WindowManager& windowManager
    );
    void Initialize(const VulkanCore::VRenderPass& renderPass);
    void BeginRender();
    void Render(VulkanCore::VCommandBuffer& commandBuffer);
    void EndRender();
    void Destroy();
private:

    const VulkanCore::VDevice& m_device;
    const VulkanCore::VulkanInstance& m_instance;
    const WindowManager& m_windowManager;

    vk::PipelineCache m_imguiPipelineCache;
    vk::DescriptorPool m_imguiDescriptorPool;

    ImGuiIO* m_io;
};

} // VEditor

#endif //IMGUIINITIALIZER_HPP
