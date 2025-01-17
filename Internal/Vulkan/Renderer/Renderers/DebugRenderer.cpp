//
// Created by wpsimon09 on 30/12/24.
//

#include "DebugRenderer.hpp"

#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace Renderer
{
    int RecordCommandBufferToDrawDebugGeometry(
        const VulkanCore::VDevice& device,
        int currentFrameIndex,
        vk::CommandBuffer commandBuffer,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
        const VulkanStructs::RenderContext& renderContext,
        const VulkanCore::VGraphicsPipeline& pipeline)
    {
        int drawCallCount = 0;
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());

        auto& dstSetDataStruct = pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        commandBuffer.bindIndexBuffer(renderContext.DrawCalls[0].meshData->indexData_BB.buffer, 0, vk::IndexType::eUint32);
        for (int i = 0; i < renderContext.DrawCalls.size(); i++)
        {
            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(i)[
                currentFrameIndex];

            auto& drawCall = renderContext.DrawCalls[i];

            std::vector<vk::Buffer> vertexBuffers = {renderContext.DrawCalls[i].meshData->vertexData_BB.buffer};
            std::vector<vk::DeviceSize> offsets = {renderContext.DrawCalls[i].meshData->vertexData_BB.offset};

            commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            commandBuffer.pushDescriptorSetWithTemplateKHR(
                pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, device.DispatchLoader);

                auto vertexOffset =  drawCall.meshData->vertexData_BB.offset / sizeof(ApplicationCore::Vertex);
                commandBuffer.drawIndexed(36, 1, 0,static_cast<uint32_t>(vertexOffset) , 0);
                drawCallCount++;
        }
        return drawCallCount;
     }

    int DrawSelectedMeshes(const VulkanCore::VDevice& device, int currentFrameIndex,
        vk::CommandBuffer commandBuffer, const VulkanUtils::VUniformBufferManager& uniformBufferManager,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager, const VulkanStructs::RenderContext& renderContext,
        const VulkanCore::VGraphicsPipeline& pipeline)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());
        auto& dstSetDataStruct = pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        int drawCallCount = 0;
        for (int i = 0; i < renderContext.DrawCalls.size(); i++)
        {

            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(renderContext.DrawCalls[i].objectID)[currentFrameIndex];

            auto& drawCall = renderContext.DrawCalls[i];

            std::vector<vk::Buffer> vertexBuffers = {renderContext.DrawCalls[i].meshData->vertexData.buffer};
            std::vector<vk::DeviceSize> offsets = {renderContext.DrawCalls[i].meshData->vertexData.offset};

            commandBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, drawCall.meshData->indexData.offset, vk::IndexType::eUint32);
            commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            commandBuffer.pushDescriptorSetWithTemplateKHR(
                pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, device.DispatchLoader);

            commandBuffer.drawIndexed(drawCall.indexCount, 1, 0, 0, 0);
            drawCallCount++;
        }
        return drawCallCount;
    }
} // Renderer
