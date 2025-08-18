//
// Created by wpsimon09 on 18/08/2025.
//

#include "LightPass.hpp"

#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Renderer/RenderingUtils.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
ForwardRender::ForwardRender(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache, int width, int height)
    : RenderPass(device, width, height)
{

    //=====================================================================
    // Forward Lit Effect (main opaque forward shader)
    //=====================================================================
    auto frowardEffect =
        std::make_unique<VulkanUtils::VRasterEffect>(device, "Forward lit", "Shaders/Compiled/BasicTriangle.vert.spv",
                                                     "Shaders/Compiled/GGXColourFragmentMultiLight.frag.spv",
                                                     descLayoutCache, EShaderBindingGroup::ForwardLit);
    frowardEffect->SetTopology(vk::PrimitiveTopology::eTriangleList);

    m_effects[EForwardRenderEffects::ForwardShader] = std::move(frowardEffect);

    //=====================================================================
    // Transparent Forward Lit (alpha blend / additive pass)
    //=====================================================================
    auto transparentEffect = std::make_unique<VulkanUtils::VRasterEffect>(
        device, "Forward lit transparent", "Shaders/Compiled/BasicTriangle.vert.spv",
        "Shaders/Compiled/GGXColourFragmentMultiLight.frag.spv", descLayoutCache, EShaderBindingGroup::ForwardLit);

    transparentEffect->SetTopology(vk::PrimitiveTopology::eTriangleList).EnableAdditiveBlending().SetDepthOpLessEqual();

    transparentEffect->EnableAdditiveBlending();

    m_effects[EForwardRenderEffects::AplhaBlend] = std::move(transparentEffect);

    //=====================================================================
    // Editor Billboards (icons, gizmos, unlit quads)
    //=====================================================================
    auto editorBillboards =
        std::make_unique<VulkanUtils::VRasterEffect>(device, "Editor billboards", "Shaders/Compiled/EditorBillboard.vert.spv",
                                                     "Shaders/Compiled/EditorBilboard.frag.spv", descLayoutCache,
                                                     EShaderBindingGroup::ForwardUnlit);

    editorBillboards->SetTopology(vk::PrimitiveTopology::eTriangleList).SetCullNone().SetVertexInputMode(EVertexInput::Position_UV);
    //.SetDepthOpLessEqual();

    m_effects[EForwardRenderEffects::EditorBilboard] = std::move(editorBillboards);

    //=====================================================================
    // Debug Lines (wireframe-style lines for debugging)
    //=====================================================================
    auto debugLine = std::make_unique<VulkanUtils::VRasterEffect>(device, "Debug lines", "Shaders/Compiled/DebugLines.vert.spv",
                                                                  "Shaders/Compiled/DebugLines.frag.spv", descLayoutCache,
                                                                  EShaderBindingGroup::ForwardUnlitNoMaterial);

    debugLine->SetTopology(vk::PrimitiveTopology::eTriangleList)
        .SetCullNone()
        .SetPolygonLine()
        .SetLineWidth(2)
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetDepthOpLessEqual();

    m_effects[EForwardRenderEffects::DebugLine] = std::move(debugLine);

    //=====================================================================
    // Object Outline Pass (stencil-based outlines)
    //=====================================================================
    auto outline = std::make_unique<VulkanUtils::VRasterEffect>(device, "Outline", "Shaders/Compiled/DebugLines.vert.spv",
                                                                "Shaders/Compiled/Outliines.frag.spv", descLayoutCache,
                                                                EShaderBindingGroup::ForwardUnlitNoMaterial);

    outline->SetStencilTestOutline().SetVertexInputMode(EVertexInput::PositionOnly).SetDepthOpAllways();

    m_effects[EForwardRenderEffects::Outline] = std::move(outline);

    //=====================================================================
    // Debug Shapes (lines, wireframe primitives, geometry helpers)
    //=====================================================================
    auto debugShapes = std::make_unique<VulkanUtils::VRasterEffect>(device, "Debug shapes", "Shaders/Compiled/DebugLines.vert.spv",
                                                                    "Shaders/Compiled/DebugGeometry.frag.spv", descLayoutCache,
                                                                    EShaderBindingGroup::ForwardUnlitNoMaterial);

    debugShapes->SetCullNone()
        .SetLineWidth(7)
        .SetPolygonLine()
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetTopology(vk::PrimitiveTopology::eLineList)
        .SetDepthOpLessEqual();

    m_effects[EForwardRenderEffects::DebugLine] = std::move(debugShapes);

    //=====================================================================
    // Skybox (environment cube rendering)
    //=====================================================================
    auto skybox = std::make_unique<VulkanUtils::VRasterEffect>(device, "Sky Box", "Shaders/Compiled/SkyBox.vert.spv",
                                                               "Shaders/Compiled/SkyBox.frag.spv", descLayoutCache,
                                                               EShaderBindingGroup::Skybox);

    skybox->SetCullNone().SetVertexInputMode(EVertexInput::PositionOnly).SetDisableDepthWrite().SetDepthOpLessEqual().DisableStencil();

    m_effects[EForwardRenderEffects::SkyBox] = std::move(skybox);
    //=====================================================================

    //======================================================================
    //************************* CREATE RENDER TARGET ***********************
    //======================================================================
    Renderer::RenderTarget2CreatInfo lightPassCI{
        width,
        height,
        true,
        false,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ResolveModeFlagBits::eAverage,
    };

    m_renderTargets.emplace_back(std::make_unique<Renderer::RenderTarget2>(m_device, lightPassCI));

    //========================================================================
    //************************* BUILD ALL EFFECT *****************************
    //========================================================================
    for(auto& e : m_effects)
    {
        e.second->GetReflectionData()->Print();
        e.second->BuildEffect();
    }
}
void ForwardRender::Init(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
{
    for(auto& effect : m_effects)
    {
        auto& e = effect.second;

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
                // std::vector<PerObjectData> SSBO.
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
        auto& e = effect.second;

        //=========================
        // for each frame in flight

          switch(e->GetBindingGroup())
            {

                case EShaderBindingGroup::ForwardLit: {

                    e->SetNumWrites(0, 6200, 0);
                    // TODO: write only so many texture as are in the view frustrum
                    e->WriteImageArray(currentFrame, 1, 1, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());

                    if(renderContext->irradianceMap)
                    {
                        e->WriteImage(currentFrame, 1, 2, renderContext->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));
                    }
                    if(renderContext->prefilterMap)
                    {
                        e->WriteImage(currentFrame, 1, 3, renderContext->prefilterMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));
                    }
                    if(renderContext->brdfMap)
                    {
                        e->WriteImage(currentFrame, 1, 4, renderContext->brdfMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
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
                        e->WriteImage(currentFrame, 1, 0, renderContext->hdrCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
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
        assert(cmdBuffer.GetIsRecording() && "Command buffer is not in recording state !");
    int drawCallCount = 0;
    //==============================================
    // CREATE RENDER PASS INFO
    //==============================================
    std::vector<vk::RenderingAttachmentInfo> colourAttachments = {
        m_renderTargets[EForwardRenderAttachments::Main]->GenerateAttachmentInfo(vk::ImageLayout::eColorAttachmentOptimal,
                                                     vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore),
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


    m_renderTargets[EForwardRenderAttachments::Main]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                                vk::ImageLayout::eShaderReadOnlyOptimal);


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

    auto  currentVertexBuffer = renderContext->drawCalls.begin()->second.vertexData;
    auto  currentIndexBuffer  = renderContext->drawCalls.begin()->second.indexData;
    auto& currentEffect       = renderContext->drawCalls.begin()->second.effect;

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
        if(drawCall.second.effect != currentEffect)
        {
            currentEffect = drawCall.second.effect;
            drawCall.second.effect->BindPipeline(cmdB);
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
        pcInfo.layout     = drawCall.second.effect->GetPipelineLayout();
        pcInfo.size       = sizeof(PerObjectPushConstant);
        pcInfo.offset     = 0;
        pcInfo.pValues    = &pc;
        pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;

        drawCall.second.effect->CmdPushConstant(cmdB, pcInfo);


        cmdB.drawIndexed(drawCall.second.indexData->size / sizeof(uint32_t), 1,
                         drawCall.second.indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                         drawCall.second.vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);


        //m_forwardRendererOutput = &m_renderTargets[EForwardRenderAttachments::Main]->GetResolvedImage();

        drawCallCount++;
    }

    cmdB.endRendering();


    m_renderTargets[EForwardRenderAttachments::Main]->TransitionAttachments(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
                                                vk::ImageLayout::eColorAttachmentOptimal);

    renderContext->depthBuffer->TransitionAttachments(cmdBuffer, vk::ImageLayout::eDepthStencilReadOnlyOptimal,
                                                vk::ImageLayout::eDepthStencilAttachmentOptimal);

    //m_renderingStatistics.DrawCallCount = drawCallCount;
}


}  // namespace Renderer
