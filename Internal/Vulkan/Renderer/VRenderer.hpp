//
// Created by wpsimon09 on 15/10/24.
//

#ifndef VRENDERER_HPP
#define VRENDERER_HPP
#include <memory>

class Client;
namespace VulkanCore
{
    class VulkanInstance;
    class VDevice;
    class VCommandBuffer;
    class VCommandPool;
    class VRenderPass;
    class VPipelineManager;
    class VSwapChain;

}

namespace Renderer {

class VRenderer {
public:
    VRenderer(const VulkanCore::VulkanInstance &instance, const VulkanCore::VDevice& device, const Client& client);
    void Init();
    void Render();
    void Destroy();
    ~VRenderer() = default;
private:
    void PrepareViewPort(int imageIndex);
private:
    const Client& m_client;
    const VulkanCore::VDevice& m_device;

    std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;
    std::unique_ptr<class VulkanCore::VPipelineManager> m_pipelineManager;
    std::unique_ptr<class VulkanCore::VRenderPass> m_mainRenderPass;
    std::unique_ptr<class VulkanCore::VCommandPool> m_renderingCommandPool;
    std::unique_ptr<class VulkanCore::VCommandBuffer> m_renderingCommandBuffer;
};

} // Renderer

#endif //VRENDERER_HPP
