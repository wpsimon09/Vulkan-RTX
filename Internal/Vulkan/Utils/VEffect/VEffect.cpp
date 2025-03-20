//
// Created by wpsimon09 on 19/03/25.
//

#include "VEffect.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace VulkanUtils
{
        VEffect::VEffect(const VulkanCore::VDevice& device, const VulkanCore::VShader& shader,
                         const Renderer::RenderTarget& effectOutput,
                         std::shared_ptr<VulkanUtils::VPushDescriptorSet>& descriptorSet):m_shader(shader), m_device(device), m_descriptorSet(descriptorSet)
        {
            m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
                device, shader, effectOutput, m_descriptorSet->GetLayout());
            m_descriptorSet->CreateDstUpdateInfo(*m_pipeline);
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
