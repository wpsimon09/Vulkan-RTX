//
// Created by wpsimon09 on 30/12/24.
//

#ifndef DEBUGRENDERER_HPP
#define DEBUGRENDERER_HPP
#include "BaseRenderer.hpp"

namespace VulkanUtils
{
    class VPushDescriptorManager;
}

namespace Renderer
{
    void RecordCommandBufferToDrawDebugGeometry(const VulkanCore::VDevice& device,
                                                int currentFrameIndex,
                                                vk::CommandBuffer commandBuffer,
                                                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                                VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                                                const VulkanStructs::RenderContext& renderContext,
                                                const VulkanCore::VGraphicsPipeline& pipeline
                                                );

    void DrawSelectedMeshes( const VulkanCore::VDevice& device,
                                int currentFrameIndex,
                                vk::CommandBuffer commandBuffer,
                                const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                                VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                                const VulkanStructs::RenderContext& renderContext,
                                const VulkanCore::VGraphicsPipeline& pipeline);

} // Renderer

#endif //DEBUGRENDERER_HPP
