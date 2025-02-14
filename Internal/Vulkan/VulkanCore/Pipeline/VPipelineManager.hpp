//
// Created by wpsimon09 on 06/10/24.
//

#ifndef VPIPELINEMANAGER_HPP
#define VPIPELINEMANAGER_HPP

#include <map>
#include <memory>
#include <vector>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"

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
            const VulkanCore::VRenderPass &renderPass, const VulkanUtils::VPushDescriptorManager &pushDescriptorSetManager
            );

        void DestroyPipelines();
        void InstantiatePipelines();

        const VGraphicsPipeline &GetPipeline(EPipelineType pipeline) const;
        const std::vector<std::reference_wrapper<const VGraphicsPipeline>> GetAllPipelines() const;

        ~VPipelineManager() = default;

    private:
        void GeneratePipelines();

    private:
        const VDevice &m_device;
        const VSwapChain &m_swapChain;
        const VRenderPass &m_renderPass;
        const VulkanUtils::VPushDescriptorManager &m_pushDescriptorSetManager;

        std::unique_ptr<VShader> m_baseShader;
        std::unique_ptr<VShader> m_rtxShader ;
        std::unique_ptr<VShader> m_debugLinesShader;
        std::unique_ptr<VShader> m_outlineShader;
        std::unique_ptr<VShader> m_multiLightShader;
        std::unique_ptr<VShader> m_editorBilboardShader;
        std::unique_ptr<VShader> m_outlineGeometryShader;   
        std::unique_ptr<VShader> m_debugGeometryShader;

        std::map<EPipelineType, std::unique_ptr<VGraphicsPipeline>> m_pipelines;
    };
}


#endif //VPIPELINEMANAGER_HPP
