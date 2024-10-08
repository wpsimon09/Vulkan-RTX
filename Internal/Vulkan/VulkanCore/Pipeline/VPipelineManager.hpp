//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VPIPELINEMANAGER_HPP
#define VPIPELINEMANAGER_HPP

#include <map>
#include <memory>
#include "Vulkan/Global/GlobalVulkanEnums.hpp"

namespace VulkanCore
{
    class VRenderPass;
    class VDevice;
    class VSwapChain;
    class VGraphicsPipeline;

    class VPipelineManager {
    public:
        VPipelineManager(const VulkanCore::VDevice &device,const VulkanCore::VSwapChain &swapChain, const VulkanCore::VRenderPass &renderPass);

        void DestoryPipelines();
        void CreatePipelines();

        ~VPipelineManager() = default;
    private:
        void GeneratePipelines();

    private:
        const VDevice &m_device;
        const VSwapChain &m_swapChain;
        const VRenderPass &m_renderPass;
        std::map<PIPELINE_TYPE, std::unique_ptr<VGraphicsPipeline>> m_pipelines;
    };
}



#endif //VPIPELINEMANAGER_HPP
