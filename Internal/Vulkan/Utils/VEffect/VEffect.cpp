//
// Created by wpsimon09 on 19/03/25.
//

#include "VEffect.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"

namespace VulkanUtils
{
        VEffect::VEffect(const VulkanCore::VDevice& device, const VulkanCore::VShader& shader,
                         const Renderer::RenderTarget& effectOutput,
                         VulkanUtils::PushDescriptorVariant& descriptorSet):m_shader(shader), m_device(device)
        {
        // Get pipeline layout from the Descriptor set
        std::visit([this, &device, &shader, &effectOutput](auto& dstSet)
        {
            using T = std::decay_t<decltype(dstSet)>;

            dstSet->CreateUpdateEntries(dstSet->GetDstStruct());
            if constexpr (std::is_same_v<T, VulkanUtils::VPushDescriptorSet<VulkanUtils::BasicDescriptorSet>>)
            {

                m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
                    device, shader, effectOutput, dstSet->GetLayout());
                m_dstStruct = dstSet->GetDstStruct();
            }
            else if constexpr (std::is_same_v<T, VulkanUtils::VPushDescriptorSet<VulkanUtils::UnlitSingleTexture>>)
            {
                m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
                    device, shader, effectOutput, dstSet->GetLayout());
                m_dstStruct = dstSet->GetDstStruct();

            }
            else if constexpr (std::is_same_v<T, VulkanUtils::VPushDescriptorSet<VulkanUtils::ForwardShadingDstSet>>)
            {
                //TODO: does not have update entries and thus gives segv....
                m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
                    device, shader, effectOutput, dstSet->GetLayout());
                m_dstStruct = dstSet->GetDstStruct();
            }
        }, descriptorSet);
    }

        void VEffect::SetDisableDepthTest()
        {
            m_pipeline->m_depthStencil.depthTestEnable = false;
        }

        void VEffect::SetLineWidth(int lineWidth)
        {
            m_pipeline->m_rasterizer.lineWidth = lineWidth;

        }

        void VEffect::SetCullFrontFace()
        {
            m_pipeline->m_rasterizer.cullMode = vk::CullModeFlagBits::eFront;
        }

        void VEffect::SetDisableDepthWrite()
        {
            m_pipeline->m_depthStencil.depthWriteEnable = false;
        }

        void VEffect::SetTopology(vk::PrimitiveTopology topology)
        {
            m_pipeline->m_inputAssembly.topology = topology;
        }

        void VEffect::SetPolygonLine()
        {
            m_pipeline->m_rasterizer.polygonMode = vk::PolygonMode::eLine;
        }

        void VEffect::SetPolygonPoint()
        {
            m_pipeline->m_rasterizer.polygonMode = vk::PolygonMode::ePoint;
        }

        void VEffect::BuildEffect()
        {

            auto pipelines =  m_device.GetDevice().createGraphicsPipelines(nullptr, m_pipeline->GetGraphicsPipelineCreateInfoStruct());
            assert(pipelines.result == vk::Result::eSuccess);
            for (auto &p : pipelines.value)
            {
                m_pipeline->SetCreatedPipeline(p);
            }
        }
} // VulkanUtils
