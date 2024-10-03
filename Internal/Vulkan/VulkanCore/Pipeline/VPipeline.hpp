//
// Created by wpsimon09 on 03/10/24.
//

#ifndef VPIPELINE_HPP
#define VPIPELINE_HPP
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VSwapChain;
    class VDevice;
    class VShader;
    class VPipeline
    {
    public:
        VPipeline(const VulkanCore::VDevice &device,const VulkanCore::VSwapChain &swapChain,const VulkanCore::VShader &shaders);

        void DestoryPipeline();
    private:
        void CreatePipeline();
    private:
        const VulkanCore::VShader &m_shaders;
        const VulkanCore::VDevice &m_device;
        const VulkanCore::VSwapChain &m_swapChain;

        vk::Pipeline m_pipeline;

    };
}


#endif //VPIPELINE_HPP
