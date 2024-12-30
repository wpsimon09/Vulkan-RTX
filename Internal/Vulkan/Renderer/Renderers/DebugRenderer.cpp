//
// Created by wpsimon09 on 30/12/24.
//

#include "DebugRenderer.hpp"

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace Renderer {


    void RecordCommandBufferToDrawDebugGeometry(
        const VulkanCore::VDevice& device,
        int currentFrameIndex,
        vk::CommandBuffer commandBuffer,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
        const VulkanStructs::RenderContext& renderContext,
        const VulkanCore::VGraphicsPipeline& pipeline)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,pipeline.GetPipelineInstance());
        auto& dstSetDataStruct = pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        for (int i = 0; i < renderContext.DrawCalls.size(); i++)
        {
            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(i)[
                currentFrameIndex];

            auto& drawCall = renderContext.DrawCalls[i];
            auto& material = drawCall.material;

            std::vector<vk::Buffer> vertexBuffers = {renderContext.DrawCalls[i].AABBVertexBuffer};
            std::vector<vk::DeviceSize> offsets = {0};

            commandBuffer.bindIndexBuffer(drawCall.indexBuffer, 0, vk::IndexType::eUint32);
            commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            commandBuffer.pushDescriptorSetWithTemplateKHR(
                pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, device.DispatchLoader);

            commandBuffer.drawIndexed(drawCall.AABBIndexCount, 1, 0, 0, 0);

        }


    }
} // Renderer