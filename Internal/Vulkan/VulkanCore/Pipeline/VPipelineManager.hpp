//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VPIPELINEMANAGER_HPP
#define VPIPELINEMANAGER_HPP

#include <map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"

namespace VulkanCore
{
    class VRenderPass;
    class VDevice;
    class VSwapChain;
    class VPipeline;

    class VPipelineManager {
    public:
        VPipelineManager(const VulkanCore::VDevice &device,const VulkanCore::VSwapChain &swapChain, const VulkanCore::VRenderPass &renderPass);

        void DestoryPipelines();
        void CreatePipelines();

        ~VPipelineManager() = default;
    private:
        std::vector<vk::GraphicsPipelineCreateInfo> ConfigurePipelines();

    private:
        const VDevice &m_device;
        const VSwapChain &m_swapChain;
        const VRenderPass &m_renderPass;
        std::map<PIPELINE_TYPE, vk::Pipeline> m_pipelines;
    };
}



#endif //VPIPELINEMANAGER_HPP
