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
                         std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& shaderResourceGroup): m_device(device), m_resourceGroup(shaderResourceGroup), m_name(name)
        {
            m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
                device, shader,m_resourceGroup->GetDescriptorSetLayout());
            m_pipeline->Init();

            m_resourceGroup->CreateDstUpdateInfo(*m_pipeline);

            m_ID = EffectIndexCounter++;
    }

        VEffect::VEffect(const VulkanCore::VDevice& device,const std::string& name, const std::string& vertex, const std::string& fragment,
            std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet):m_device(device), m_resourceGroup(descriptorSet), m_name(name), m_shader(std::in_place, device, vertex, fragment)
        {
            m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(
               device, m_shader.value(), m_resourceGroup->GetDescriptorSetLayout());
            m_pipeline->Init();

            m_resourceGroup->CreateDstUpdateInfo(*m_pipeline);

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

        VEffect& VEffect::SetDepthOpEqual()
        {
            m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eEqual;
            return *this;
        }

        VEffect& VEffect::SetDepthOpLessEqual()
        {
            m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
            return *this;
        }

        VEffect& VEffect::SetFrontFaceClockWise()
        {
            m_pipeline->m_rasterizer.frontFace = vk::FrontFace::eClockwise;
            return *this;
        }

        VEffect& VEffect::SetVertexInputMode(EVertexInput inputMode)
        {
            m_pipeline->CreateVertexInputBindingAndAttributes(inputMode);
            return *this;
        }

        VEffect& VEffect::SetStencilTestOutline()
        {
            m_pipeline->m_depthStencil.back.compareOp = vk::CompareOp::eNotEqual;
            m_pipeline->m_depthStencil.back.failOp = vk::StencilOp::eKeep;
            m_pipeline->m_depthStencil.back.depthFailOp = vk::StencilOp::eKeep;
            m_pipeline->m_depthStencil.back.passOp = vk::StencilOp::eReplace;
            m_pipeline->m_depthStencil.back.reference = 1;



            m_pipeline->m_depthStencil.front = m_pipeline->m_depthStencil.back;

            return *this;

        }

        VEffect& VEffect::DisableStencil()
        {
            m_pipeline->m_depthStencil.stencilTestEnable = vk::False;
            return  *this;
        }

        VEffect& VEffect::SetDepthTestNever()
        {
            m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eNever;
            return *this;
        }

        VEffect& VEffect::SetColourOutputFormat(vk::Format format)
        {
            m_pipeline->SetColourOutputFormat(format);
            return *this;
        }

        VEffect& VEffect::SetPiplineNoMultiSampling()
        {
            m_pipeline->m_multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
            return *this;
        }

        VEffect& VEffect::SetNullVertexBinding()
        {
            m_pipeline->m_vertexInputState.vertexAttributeDescriptionCount = 0;
            m_pipeline->m_vertexInputState.vertexBindingDescriptionCount = 0;
            m_pipeline->m_vertexInputState.pVertexBindingDescriptions = nullptr;
            m_pipeline->m_vertexInputState.pVertexAttributeDescriptions = nullptr;
            return *this;
        }

        VEffect& VEffect::DissableFragmentWrite()
        {
            m_pipeline->m_rasterizer.rasterizerDiscardEnable = vk::True;
            return *this;
        }

        VEffect& VEffect::SetDepthOpLess()
        {
            m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eLess;
            return  *this;
        }


        std::string& VEffect::GetName()
        {
            return m_name;
        }

        DescriptorSetTemplateVariant& VEffect::GetResrouceGroupStructVariant()
        {
            return m_resourceGroup->GetResourceGroupStruct();
        }

        void VEffect::BuildEffect()
        {

            auto pipelines =  m_device.GetDevice().createGraphicsPipelines(nullptr, m_pipeline->GetGraphicsPipelineCreateInfoStruct());
            assert(pipelines.result == vk::Result::eSuccess);
            for (auto &p : pipelines.value)
            {
                m_pipeline->SetCreatedPipeline(p);
            }
            m_shader->DestroyExistingShaderModules();
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
            return m_resourceGroup->GetUpdateTemplate();
        }

        unsigned short VEffect::EvaluateRenderingOrder()
        {
            return 0;
        }

        int& VEffect::GetID()
        {
            return m_ID;
        }

        EDescriptorLayoutStruct VEffect::GetLayoutStructType()
        {
            return m_resourceGroup->GetResourceGroupStrucutureType();
        }
} // VulkanUtils
