//
// Created by wpsimon09 on 30/12/24.
//

#include "DebugRenderer.hpp"

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer
{
    int RecordCommandBufferToDrawDebugGeometry(
        const VulkanCore::VDevice& device,
        int currentFrameIndex,
        vk::CommandBuffer commandBuffer,
        const VulkanUtils::VUniformBufferManager& uniformBufferManager,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
        std::vector<VulkanStructs::DrawCallData>& drawCalls,
        const VulkanCore::VGraphicsPipeline& pipeline)
    {
        int drawCallCount = 0;

        if (drawCalls.empty())
            return 0;

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());

        auto& dstSetDataStruct = pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        commandBuffer.bindIndexBuffer(drawCalls[0].meshData->indexData_BB.buffer, 0, vk::IndexType::eUint32);
        for (int i = 0; i < drawCalls.size(); i++)
        {
            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(drawCalls[i].drawCallID)[
                currentFrameIndex];

            auto& drawCall = drawCalls[i];

            std::vector<vk::Buffer> vertexBuffers = {drawCalls[i].meshData->vertexData_BB.buffer};
            std::vector<vk::DeviceSize> offsets = {drawCalls[i].meshData->vertexData_BB.offset};

            commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            /*commandBuffer.pushDescriptorSetWithTemplateKHR(
                pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, device.DispatchLoader);*/

            auto vertexOffset = 0;
            commandBuffer.drawIndexed(drawCall.indexCount_BB, 1, 0, static_cast<uint32_t>(vertexOffset), 0);
            drawCallCount++;
        }
        return drawCallCount;
    }

    int DrawSelectedMeshes(const VulkanCore::VDevice& device, int currentFrameIndex,
                           vk::CommandBuffer commandBuffer,
                           const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                           VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                           std::vector<VulkanStructs::DrawCallData>& drawCalls,
                           const VulkanCore::VGraphicsPipeline& pipeline)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());
        auto& dstSetDataStruct = pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        int drawCallCount = 0;
        for (int i = 0; i < drawCalls.size(); i++)
        {
            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(
                drawCalls[i].drawCallID)[currentFrameIndex];

            auto& drawCall = drawCalls[i];

            std::vector<vk::Buffer> vertexBuffers = {drawCalls[i].meshData->vertexData.buffer};
            std::vector<vk::DeviceSize> offsets = {drawCalls[i].meshData->vertexData.offset};

            commandBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, drawCall.meshData->indexData.offset,
                                          vk::IndexType::eUint32);
            commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            /*commandBuffer.pushDescriptorSetWithTemplateKHR(
                pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, device.DispatchLoader);*/

            commandBuffer.drawIndexed(drawCall.indexCount, 1, 0, 0, 0);
            drawCallCount++;
        }
        return drawCallCount;
    }

    int DrawEditorBillboards(const VulkanCore::VDevice& device, int currentFrameIndex, vk::CommandBuffer commandBuffer,
                             const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                             VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                             std::vector<VulkanStructs::DrawCallData>& drawCalls,
                             const VulkanCore::VGraphicsPipeline& pipeline)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());
        auto& dstSetDataStruct = pushDescriptorManager.GetDescriptorSetDataStruct();
        dstSetDataStruct.cameraUBOBuffer = uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex];

        int drawCallCount = 0;
        for (int i = 0; i < drawCalls.size(); i++)
        {
            dstSetDataStruct.meshUBBOBuffer = uniformBufferManager.GetPerObjectDescriptorBufferInfo(
                drawCalls[i].drawCallID)[currentFrameIndex];
            auto& drawCall = drawCalls[i];

            //sets albedo texture of the image
            dstSetDataStruct.diffuseTextureImage = drawCall.material->GetTexture(ETextureType::Diffues)->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
            std::vector<vk::Buffer> vertexBuffers = {drawCalls[i].meshData->vertexData.buffer};
            std::vector<vk::DeviceSize> offsets = {drawCalls[i].meshData->vertexData.offset};

            commandBuffer.bindIndexBuffer(drawCall.meshData->indexData.buffer, drawCall.meshData->indexData.offset,
                                          vk::IndexType::eUint32);
            commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

            /*commandBuffer.pushDescriptorSetWithTemplateKHR(
                pushDescriptorManager.GetTemplate(),
                pipeline.GetPipelineLayout(), 0,
                dstSetDataStruct, device.DispatchLoader);*/

            commandBuffer.drawIndexed(drawCall.indexCount, 1, 0, 0, 0);
            drawCallCount++;
        }
        return drawCallCount;
    }
} // Renderer
