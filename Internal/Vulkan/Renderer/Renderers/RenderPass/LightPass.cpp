//
// Created by wpsimon09 on 18/08/2025.
//

#include "LightPass.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
ForwardRender::ForwardRender(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectLibrary, int width, int height)
    : RenderPass(device, width, height)
{

    m_effects.resize(EForwardRenderEffects::ForwardRenderEffectsCount);
    //=====================================================================
    // Forward Lit Effect (main opaque forward shader)
    //=====================================================================
    m_effects[EForwardRenderEffects::ForwardShader] =
        effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::ForwardShader);

    //=====================================================================
    // Transparent Forward Lit (alpha blend / additive pass)
    //=====================================================================
    m_effects[EForwardRenderEffects::AplhaBlend] =
        effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::AplhaBlend);
    ;

    //=====================================================================
    // Editor Billboards (icons, gizmos, unlit quads)
    //=====================================================================
    m_effects[EForwardRenderEffects::EditorBilboard] =
        effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::EditorBilboard);

    //=====================================================================
    // Debug Lines (wireframe-style lines for debugging)
    //=====================================================================
    m_effects[EForwardRenderEffects::DebugLine] =
        effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::DebugLine);

    //=====================================================================
    // Object Outline Pass (stencil-based outlines)
    //=====================================================================
    m_effects[EForwardRenderEffects::Outline] =
        effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::Outline);

    //=====================================================================
    // Skybox (environment cube rendering)
    //=====================================================================
    m_effects[EForwardRenderEffects::SkyBox] =
        effectLibrary.GetEffect<VulkanUtils::VRasterEffect>(ApplicationCore::EEffectType::SkyBox);
    //=====================================================================

    //======================================================================
    //************************* CREATE RENDER TARGET ***********************
    //======================================================================
    Renderer::RenderTarget2CreatInfo lightPassCI{width,
                                                 height,
                                                 true,
                                                 false,
                                                 vk::Format::eR16G16B16A16Sfloat,
                                                 vk::ImageLayout::eShaderReadOnlyOptimal,
                                                 vk::ResolveModeFlagBits::eAverage,
                                                 false,
                                                 "Forward render light pass output attachemnt"};

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, lightPassCI));
}
void ForwardRender::Init(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    for(auto& effect : m_effects)
    {
        auto& e = effect;
        if(!e)
        {
            continue;
        }

        //=========================
        // for each frame in flight

        switch(e->GetBindingGroup())
        {
            case EShaderBindingGroup::ForwardUnlitNoMaterial: {

                e->SetNumWrites(3, 0, 1);

                //========================
                // global data
                e->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame]);

                //========================
                // per model data
                e->WriteBuffer(currentFrame, 0, 1, uniformBufferManager.GetPerObjectBuffer(currentFrame));

                break;
            }

            case EShaderBindingGroup::ForwardLit: {

                e->SetNumWrites(7, 5, 0);
                //===================================
                // global data
                e->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame]);

                //===================================
                // materials
                e->WriteBuffer(currentFrame, 0, 1, uniformBufferManager.GetPerObjectBuffer(currentFrame));

                //===================================
                // lighting information
                e->WriteBuffer(currentFrame, 0, 2, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrame));

                //===================================
                // std::vector<PerObjectData> SSBO.
                e->WriteBuffer(currentFrame, 0, 3, uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrame]);

                // visibility buffer.
                e->WriteImage(currentFrame, 0, 4,
                              renderContext->visibilityBuffer->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

                // ltc
                e->WriteImage(currentFrame, 0, 5,
                              MathUtils::LookUpTables.LTC->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

                // ltc inverse
                e->WriteImage(currentFrame, 0, 6,
                              MathUtils::LookUpTables.LTCInverse->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

                e->WriteImage(currentFrame, 0, 7,
                              renderContext->transmitanceLut->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));

                e->WriteImage(currentFrame, 0, 8,
                              renderContext->aoOcclusionMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));


                break;
            }

            case EShaderBindingGroup::ForwardUnlit: {
                e->SetNumWrites(7, 4, 0);
                //===================================
                // global data
                e->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame]);

                //===================================
                // materials
                e->WriteBuffer(currentFrame, 0, 1, uniformBufferManager.GetPerObjectBuffer(currentFrame));

                //===================================
                // std::vector<PerObjectData> SSBO.g
                e->WriteBuffer(currentFrame, 0, 2, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrame));

                break;
            }
            case EShaderBindingGroup::Skybox: {
                e->SetNumWrites(1, 0);

                //====================================
                // global data
                e->WriteBuffer(currentFrame, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrame]);
                break;
            }
        }
        e->ApplyWrites(currentFrame);
    }
}

void ForwardRender::Update(int                                   currentFrame,
                           VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                           VulkanUtils::RenderContext*           renderContext,
                           VulkanStructs::PostProcessingContext* postProcessingContext)
{
    for(auto& effect : m_effects)
    {
        auto& e = effect;

        //=========================
        // for each frame in flight
        if(!e)
        {
            continue;
        }
        switch(e->GetBindingGroup())
        {

            case EShaderBindingGroup::ForwardLit: {

                e->SetNumWrites(0, 6200, 0);

                // TODO: write only so many texture as are in the view frustrum
                e->WriteImageArray(currentFrame, 1, 1, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());

                if(renderContext->irradianceMap)
                {
                    e->WriteImage(currentFrame, 1, 2,
                                  renderContext->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));
                }
                if(renderContext->prefilterMap)
                {
                    e->WriteImage(currentFrame, 1, 3,
                                  renderContext->prefilterMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));
                }
                if(renderContext->brdfMap)
                {
                    e->WriteImage(currentFrame, 1, 4,
                                  renderContext->brdfMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                }
                break;
            }
            case EShaderBindingGroup::ForwardUnlit: {
                e->SetNumWrites(0, 5600, 0);
                e->WriteImageArray(currentFrame, 1, 0, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());
                break;
            }
            case EShaderBindingGroup::ForwardUnlitNoMaterial: {
                break;
            }
            case EShaderBindingGroup::Skybox: {
                //====================================
                // global data
                e->SetNumWrites(0, 1);
                if(renderContext->hdrCubeMap)
                {
                    e->WriteImage(currentFrame, 1, 0,
                                  renderContext->hdrCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                }
                break;
            }

            default: {
                // throw std::runtime_error("Unsupported bindinggroup !");
                break;
            }
        }

        e->ApplyWrites(currentFrame);
    }
}

void ForwardRender::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
    int drawCallCount = 0;
    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================
    std::vector<vk::RenderingAttachmentInfo> colourAttachments = {
        m_renderTargets[EForwardRenderAttachments::Main]->GenerateAttachmentInfo(
            vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
    };

    auto depthAttachment =
        renderContext->depthBuffer->GenerateAttachmentInfo(vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                                           vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore);

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = colourAttachments.size();
    renderingInfo.pColorAttachments    = colourAttachments.data();

    renderingInfo.pDepthAttachment   = &depthAttachment;
    renderingInfo.pStencilAttachment = &depthAttachment;


    //==============================================
    // START RENDER PASS
    //==============================================
    auto& cmdB = cmdBuffer.GetCommandBuffer();


    VulkanUtils::VBarrierPosition barrierPos = {vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                                                vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                vk::AccessFlagBits2::eColorAttachmentWrite};
    m_renderTargets[EForwardRenderAttachments::Main]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eAttachmentOptimal,
                                                                            vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);


    cmdB.beginRendering(&renderingInfo);


    // if there is nothing to render end the render process
    if(renderContext->drawCalls.empty())
    {
        cmdB.endRendering();
        //m_renderingStatistics.DrawCallCount = drawCallCount;
        return;
    }
    //=================================================
    // UPDATE DESCRIPTOR SETS
    //=================================================

    auto currentVertexBuffer = renderContext->drawCalls.begin()->second.vertexData;
    auto currentIndexBuffer  = renderContext->drawCalls.begin()->second.indexData;
    auto currentEffect       = m_effects[(EForwardRenderEffects)renderContext->drawCalls.begin()->second.effect];

    vk::DeviceSize indexBufferOffset = 0;

    cmdB.bindVertexBuffers(0, {currentVertexBuffer->buffer}, {0});
    cmdB.bindIndexBuffer(currentIndexBuffer->buffer, 0, vk::IndexType::eUint32);

    //============================================
    // CONFIGURE VIEW PORT
    //===============================================
    Renderer::ConfigureViewPort(cmdB, m_width, m_height);

    //=================================================
    // RECORD OPAQUE DRAW CALLS
    //=================================================
    currentEffect->BindPipeline(cmdB);
    currentEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);

    for(auto& drawCall : renderContext->drawCalls)
    {
        if(drawCall.second.postProcessingEffect)
        {
            continue;
        }
        auto& material = drawCall.second.material;
        if(m_effects[static_cast<EForwardRenderEffects>(drawCall.second.effect)] != currentEffect)
        {
            currentEffect = m_effects[static_cast<EForwardRenderEffects>(drawCall.second.effect)];
            currentEffect->BindPipeline(cmdB);
            currentEffect->BindDescriptorSet(cmdBuffer.GetCommandBuffer(), currentFrame, 0);
        }

        if(drawCall.second.selected)
            cmdB.setStencilTestEnable(true);
        else
            cmdB.setStencilTestEnable(false);

        //================================================================================================
        // BIND VERTEX BUFFER ONLY IF IT HAS CHANGED
        //================================================================================================
        if(currentVertexBuffer->BufferID != drawCall.second.vertexData->BufferID)
        {
            auto firstBinding = 0;

            std::vector<vk::Buffer>     vertexBuffers = {drawCall.second.vertexData->buffer};
            std::vector<vk::DeviceSize> offsets       = {0};
            vertexBuffers                             = {drawCall.second.vertexData->buffer};
            cmdB.bindVertexBuffers(firstBinding, vertexBuffers, offsets);
            currentVertexBuffer = drawCall.second.vertexData;
        }

        if(currentIndexBuffer->BufferID != drawCall.second.indexData->BufferID)
        {
            indexBufferOffset = 0;
            cmdB.bindIndexBuffer(drawCall.second.indexData->buffer, 0, vk::IndexType::eUint32);
            currentIndexBuffer = drawCall.second.indexData;
        }


        PerObjectPushConstant pc{};
        pc.indexes.x   = drawCall.second.drawCallID;
        pc.modelMatrix = drawCall.second.modelMatrix;

        vk::PushConstantsInfo pcInfo;
        pcInfo.layout     = m_effects[static_cast<EForwardRenderEffects>(drawCall.second.effect)]->GetPipelineLayout();
        pcInfo.size       = sizeof(PerObjectPushConstant);
        pcInfo.offset     = 0;
        pcInfo.pValues    = &pc;
        pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

        m_effects[(EForwardRenderEffects)drawCall.second.effect]->CmdPushConstant(cmdB, pcInfo);


        cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                         drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                         drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);


        //m_forwardRendererOutput = &m_renderTargets[EForwardRenderAttachments::Main]->GetResolvedImage();

        drawCallCount++;
    }

    cmdB.endRendering();

    //TODO: this is being transitioned but the FOG pass (not yet) will draw to this as well as atmosphere pass, maybe it is better to keep the transition at the very end
    if(!renderContext->atmosphereCall.has_value())
    {
        // transition of the colour attachemtn will be done in atmosphere / fog draw call
        m_renderTargets[EForwardRenderAttachments::Main]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                                                vk::ImageLayout::eColorAttachmentOptimal,
                                                                                barrierPos.Switch());
    }
    else
    {
        // put barrier here so that atmosphere has to wait until the colour of the forward is available
        m_renderTargets[EForwardRenderAttachments::Main]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                                                vk::ImageLayout::eColorAttachmentOptimal,
                                                                                barrierPos.Switch());
    }

    barrierPos = {vk::PipelineStageFlagBits2::eColorAttachmentOutput | vk::PipelineStageFlagBits2::eLateFragmentTests,
                  vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                  vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
                  vk::AccessFlagBits2::eShaderRead};
    renderContext->depthBuffer->TransitionAttachments(cmdBuffer, vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                                                      vk::ImageLayout::eDepthStencilAttachmentOptimal, barrierPos);

    //m_renderingStatistics.DrawCallCount = drawCallCount;
}

void ForwardRender::Destroy()
{
    RenderPass::Destroy();
    for(int i = 0; i < m_effects.size(); i++)
    {
        if(m_effects[i])
        {
            m_effects[i]->Destroy();
        }
    }
}


}  // namespace Renderer
