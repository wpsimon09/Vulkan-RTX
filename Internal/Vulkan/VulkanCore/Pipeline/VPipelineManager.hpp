//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VPIPELINEMANAGER_HPP
#define VPIPELINEMANAGER_HPP

#include <map>
#include <memory>
#include <vector>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"

namespace VulkanUtils
{
    class VDescriptorSetManager;
}

namespace VulkanCore
{
    class VShader;
    class VRenderPass;
    class VDevice;
    class VSwapChain;
    class VGraphicsPipeline;

    class VPipelineManager
    {
    public:
        VPipelineManager(
            const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain,
            const VulkanCore::VRenderPass &renderPass, VulkanUtils::VDescriptorSetManager &descriptorSetManager
            );

        void DestroyPipelines();
        void InstantiatePipelines();

        const VGraphicsPipeline &GetPipeline(PIPELINE_TYPE pipeline) const;
        const std::vector<std::reference_wrapper<const VGraphicsPipeline>> GetAllPipelines() const;

        ~VPipelineManager() = default;

    private:
        void GeneratePipelines();

    private:
        const VDevice &m_device;
        const VSwapChain &m_swapChain;
        const VRenderPass &m_renderPass;
        VulkanUtils::VDescriptorSetManager &m_descriptorSetManager;

        std::unique_ptr<VShader> m_baseShader;
        std::map<PIPELINE_TYPE, std::unique_ptr<VGraphicsPipeline>> m_pipelines;
    };
}


#endif //VPIPELINEMANAGER_HPP
