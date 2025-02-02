//
// Created by wpsimon09 on 30/12/24.
//

#ifndef DEBUGRENDERER_HPP
#define DEBUGRENDERER_HPP
#include "BaseRenderer.hpp"

namespace VulkanStructs
{
    struct DrawCallData;
}

namespace VulkanUtils
{
    class VPushDescriptorManager;
}

namespace Renderer
{
    int RecordCommandBufferToDrawDebugGeometry(const VulkanCore::VDevice& device,
                                                int currentFrameIndex,
                                                vk::CommandBuffer commandBuffer,
                                                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                                VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                                                std::vector<VulkanStructs::DrawCallData>& drawCalls,
                                                const VulkanCore::VGraphicsPipeline& pipeline
                                                );

    int DrawSelectedMeshes( const VulkanCore::VDevice& device,
                                int currentFrameIndex,
                                vk::CommandBuffer commandBuffer,
                                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                                std::vector<VulkanStructs::DrawCallData>& drawCalls,
                                const VulkanCore::VGraphicsPipeline& pipeline);


    int DrawEditorBillboards( const VulkanCore::VDevice& device,
                                int currentFrameIndex,
                                vk::CommandBuffer commandBuffer,
                                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                                std::vector<VulkanStructs::DrawCallData>& drawCalls,
                                const VulkanCore::VGraphicsPipeline& pipeline);

} // Renderer

#endif //DEBUGRENDERER_HPP
