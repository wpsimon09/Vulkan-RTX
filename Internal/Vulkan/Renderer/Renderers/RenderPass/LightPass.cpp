//
// Created by wpsimon09 on 18/08/2025.
//

#include "LightPass.hpp"

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
void ForwardRender::Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext)
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
                e->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

                //========================
                // per model data
                e->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetPerObjectBuffer(currentFrameIndex));

                break;
            }

            case EShaderBindingGroup::ForwardLit: {

                e->SetNumWrites(7, 5, 0);
                //===================================
                // global data
                e->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

                //===================================
                // materials
                e->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetPerObjectBuffer(currentFrameIndex));

                //===================================
                // lighting information
                e->WriteBuffer(currentFrameIndex, 0, 2, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrameIndex));

                //===================================
                // std::vector<PerObjectData> SSBO.
                e->WriteBuffer(currentFrameIndex, 0, 3, uniformBufferManager.GetLightBufferDescriptorInfo()[currentFrameIndex]);

                break;
            }

            case EShaderBindingGroup::ForwardUnlit: {
                e->SetNumWrites(7, 4, 0);
                //===================================
                // global data
                e->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);

                //===================================
                // materials
                e->WriteBuffer(currentFrameIndex, 0, 1, uniformBufferManager.GetPerObjectBuffer(currentFrameIndex));

                //===================================
                // std::vector<PerObjectData> SSBO.
                e->WriteBuffer(currentFrameIndex, 0, 2, uniformBufferManager.GetMaterialDescriptionBuffer(currentFrameIndex));
                break;
            }
            case EShaderBindingGroup::Skybox: {
                e->SetNumWrites(1, 0);

                //====================================
                // global data
                e->WriteBuffer(currentFrameIndex, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[currentFrameIndex]);
                break;
            }
        }
        e->ApplyWrites(currentFrameIndex);
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

        e->ApplyWrites(currentFrameIndex);
    }
}

void ForwardRender::Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext)
{
}


}  // namespace Renderer
