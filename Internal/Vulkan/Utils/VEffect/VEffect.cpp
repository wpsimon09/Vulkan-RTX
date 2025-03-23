//
// Created by wpsimon09 on 19/03/25.
//

#include "VEffect.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace VulkanUtils
{
        VEffect::VEffect(const VulkanCore::VDevice& device,const std::string& name, const VulkanCore::VShader& shader,
                         std::shared_ptr<VulkanUtils::VPushDescriptorSet>& descriptorSet): m_device(device), m_descriptorSet(descriptorSet), m_name(name)
        {
            m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
                device, shader,m_descriptorSet->GetLayout());
            m_pipeline->Init();

            m_descriptorSet->CreateDstUpdateInfo(*m_pipeline);

            m_ID = EffectIndexCounter++;
    }

        VEffect::VEffect(const VulkanCore::VDevice& device,const std::string& name, const std::string& vertex, const std::string& fragment,
            std::shared_ptr<VulkanUtils::VPushDescriptorSet>& descriptorSet):m_device(device), m_descriptorSet(descriptorSet), m_name(name), shader(std::in_place, device, vertex, fragment)
        {
            m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
               device, shader.value(), m_descriptorSet->GetLayout());
            m_pipeline->Init();

            m_descriptorSet->CreateDstUpdateInfo(*m_pipeline);

            m_ID = EffectIndexCounter++;
        }

        VEffect& VEffect::SetDisableDepthTest()
        {
            m_pipeline->m_depthStencil.depthTestEnable = false;
            return *this;
        }

        VEffect& VEffect::SetLineWidth(int lineWidth)
        {
            m_pipeline->m_rasterizer.lineWidth = lineWidth;
            return *this;

        }

        VEffect& VEffect::SetCullFrontFace()
        {
            m_pipeline->m_rasterizer.cullMode = vk::CullModeFlagBits::eFront;
            return *this;

        }

        VEffect& VEffect::SetCullNone()
        {
            m_pipeline->SetCullMode(vk::CullModeFlagBits::eNone);
            return *this;
        }

        VEffect& VEffect::SetDisableDepthWrite()
        {
            m_pipeline->m_depthStencil.depthWriteEnable = false;
            return *this;
        }

        VEffect& VEffect::SetTopology(vk::PrimitiveTopology topology)
        {
            m_pipeline->m_inputAssembly.topology = topology;
            return *this;
        }

        VEffect& VEffect::SetPolygonLine()
        {
            m_pipeline->m_rasterizer.polygonMode = vk::PolygonMode::eLine;
            return *this;

        }

        VEffect& VEffect::SetPolygonPoint()
        {
            m_pipeline->m_rasterizer.polygonMode = vk::PolygonMode::ePoint;
            return *this;
        }

        VEffect& VEffect::EnableAdditiveBlending()
        {
            m_pipeline->EnableBlendingAdditive();
            return *this;
        }

        VEffect& VEffect::OutputHDR()
        {
            std::vector<vk::Format> formats = {vk::Format::eR16G16B16A16Sfloat};
            m_pipeline->m_outputFormats = formats;
            return *this;
        }

        std::string& VEffect::GetName()
        {
            return m_name;
        }

        DescriptorSetTemplateVariant& VEffect::GetEffectUpdateStruct()
        {
            return m_descriptorSet->GetDstStruct();
        }

        void VEffect::BuildEffect()
        {

            auto pipelines =  m_device.GetDevice().createGraphicsPipelines(nullptr, m_pipeline->GetGraphicsPipelineCreateInfoStruct());
            assert(pipelines.result == vk::Result::eSuccess);
            for (auto &p : pipelines.value)
            {
                m_pipeline->SetCreatedPipeline(p);
            }
            shader->DestroyExistingShaderModules();
        }

        vk::PipelineLayout VEffect::GetPipelineLayout()
        {
            return m_pipeline->GetPipelineLayout();
        }

        void VEffect::BindPipeline(const vk::CommandBuffer& cmdBuffer)
        {
            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipelineInstance());
        }


        void VEffect::Destroy()
        {
            m_pipeline->Destroy();
        }

        vk::DescriptorUpdateTemplate& VEffect::GetUpdateTemplate()
        {
            return m_descriptorSet->GetUpdateTemplate();
        }

        unsigned short VEffect::EvaluateRenderingOrder()
        {
            return 0;
        }

        int& VEffect::GetID()
        {
            return m_ID;
        }
} // VulkanUtils
