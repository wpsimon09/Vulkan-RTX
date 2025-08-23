//
// Created by wpsimon09 on 20/03/25.
//

#include "EffectsLibrary.hpp"

#include "Application/Utils/LookUpTables.hpp"

#include <cstddef>
#include <memory>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/ForwardRenderer.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VShaderStorageBuffer.hpp"

namespace ApplicationCore {
EffectsLibrary::EffectsLibrary(const VulkanCore::VDevice&           device,
                               VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                               VulkanUtils::VRayTracingDataManager& rtxDataManager,
                               VulkanCore::VDescriptorLayoutCache&  descLayoutCache)
    : m_descLayoutCache(descLayoutCache)
{
    auto frowardEffect =
        std::make_shared<VulkanUtils::VRasterEffect>(device, "Forward lit", "Shaders/Compiled/BasicTriangle.vert.spv",
                                                     "Shaders/Compiled/GGXColourFragmentMultiLight.frag.spv",
                                                     descLayoutCache, EShaderBindingGroup::ForwardLit);
    frowardEffect->SetTopology(vk::PrimitiveTopology::eTriangleList);
    if(GlobalVariables::RenderingOptions::PreformDepthPrePass)
    {
        frowardEffect->SetDisableDepthWrite();
    }


    effects[EEffectType::ForwardShader] = std::move(frowardEffect);


    //==============================================================================

    auto transparentEffect = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Forward lit transparent", "Shaders/Compiled/BasicTriangle.vert.spv",
        "Shaders/Compiled/GGXColourFragmentMultiLight.frag.spv", descLayoutCache, EShaderBindingGroup::ForwardLit);

    transparentEffect->SetTopology(vk::PrimitiveTopology::eTriangleList).EnableAdditiveBlending().SetDepthOpLessEqual();

    if(GlobalVariables::RenderingOptions::PreformDepthPrePass)
    {
        transparentEffect->SetDisableDepthWrite();
    }

    effects[EEffectType::AplhaBlend] = std::move(transparentEffect);

    //==============================================================================

    auto editorBillboards =
        std::make_shared<VulkanUtils::VRasterEffect>(device, "Editor billboards", "Shaders/Compiled/EditorBillboard.vert.spv",
                                                     "Shaders/Compiled/EditorBilboard.frag.spv", descLayoutCache,
                                                     EShaderBindingGroup::ForwardUnlit);

    editorBillboards->SetTopology(vk::PrimitiveTopology::eTriangleList).SetCullNone().SetVertexInputMode(EVertexInput::Position_UV)
        //.SetDepthOpLessEqual()
        ;


    effects[EEffectType::EditorBilboard] = std::move(editorBillboards);

    //==============================================================================

    auto debugLine = std::make_shared<VulkanUtils::VRasterEffect>(device, "Debug lines", "Shaders/Compiled/DebugLines.vert.spv",
                                                                  "Shaders/Compiled/DebugLines.frag.spv",
                                                                  descLayoutCache, ForwardUnlitNoMaterial);

    debugLine->SetTopology(vk::PrimitiveTopology::eTriangleList)
        .SetCullNone()
        .SetPolygonLine()
        .SetLineWidth(2)
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetDepthOpLessEqual();


    effects[EEffectType::DebugLine] = std::move(debugLine);

    //==============================================================================

    auto outline = std::make_shared<VulkanUtils::VRasterEffect>(device, "Outline", "Shaders/Compiled/DebugLines.vert.spv",
                                                                "Shaders/Compiled/Outliines.frag.spv", descLayoutCache,
                                                                EShaderBindingGroup::ForwardUnlitNoMaterial);

    outline
        //->SetC()
        ->SetStencilTestOutline()
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetDepthOpAllways();


    effects[EEffectType::Outline] = std::move(outline);

    //===============================================================================

    auto debugShapes = std::make_shared<VulkanUtils::VRasterEffect>(device, "Debug shapes", "Shaders/Compiled/DebugLines.vert.spv",
                                                                    "Shaders/Compiled/DebugGeometry.frag.spv", descLayoutCache,
                                                                    EShaderBindingGroup::ForwardUnlitNoMaterial);

    debugShapes->SetCullNone()
        .SetLineWidth(7)
        .SetPolygonLine()
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetTopology(vk::PrimitiveTopology::eLineList)
        .SetDepthOpLessEqual();
    if(GlobalVariables::RenderingOptions::PreformDepthPrePass)
    {
        debugShapes->SetDisableDepthWrite();
    }

    effects[EEffectType::DebugLine] = std::move(debugShapes);

    //===============================================================================

    auto skybox = std::make_shared<VulkanUtils::VRasterEffect>(device, "Sky Box", "Shaders/Compiled/SkyBox.vert.spv",
                                                               "Shaders/Compiled/SkyBox.frag.spv", descLayoutCache,
                                                               EShaderBindingGroup::Skybox);


    skybox->SetCullNone().SetVertexInputMode(EVertexInput::PositionOnly).SetDisableDepthWrite().SetDepthOpLessEqual().DisableStencil();

    effects[EEffectType::SkyBox] = std::move(skybox);

    //===============================================================================
    auto depthPrePass =
        std::make_shared<VulkanUtils::VRasterEffect>(device, "Depth-PrePass effect", "Shaders/Compiled/DepthPrePass.vert.spv",
                                                     "Shaders/Compiled/DepthPrePass.frag.spv", descLayoutCache,
                                                     EShaderBindingGroup::ForwardUnlitNoMaterial, 2);
    depthPrePass->SetVertexInputMode(EVertexInput::Position_Normal).SetDepthOpLess();

    depthPrePass->AddColourAttachmentFormat(vk::Format::eR16G16B16A16Sfloat);

    effects[EEffectType::DepthPrePass] = std::move(depthPrePass);

    //===============================================================================
    VulkanCore::RTX::RTXShaderPaths rtxShaderPaths;
    rtxShaderPaths.rayGenPath     = "Shaders/Compiled/SimpleRayTracing.rgen.spv";
    rtxShaderPaths.missPath       = "Shaders/Compiled/SimpleRayTracing.miss.spv";
    rtxShaderPaths.missShadowPath = "Shaders/Compiled/SimpleRayTracing.miss2.spv";
    rtxShaderPaths.rayHitPath     = "Shaders/Compiled/SimpleRayTracing.chit.spv";
    auto rayTracingEffect =
        std::make_shared<VulkanUtils::VRayTracingEffect>(device, rtxShaderPaths, "Ray tracing ", m_descLayoutCache);

    effects[EEffectType::RayTracing] = std::move(rayTracingEffect);

    //===============================================================================
    auto rtShadowPass = std::make_shared<VulkanUtils::VRasterEffect>(device, "Ray traced shadow map effect",
                                                                     "Shaders/Compiled/RTShadowPass.vert.spv",
                                                                     "Shaders/Compiled/RTShadowPass.frag.spv",
                                                                     descLayoutCache, EShaderBindingGroup::ShadowRT);
    rtShadowPass->SetDisableDepthTest()
        .DisableStencil()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
        .SetPiplineNoMultiSampling();

    effects[EEffectType::RTShadowPass] = std::move(rtShadowPass);

    //===============================================================================
    auto toneMappingPass =
        std::make_shared<VulkanUtils::VRasterEffect>(device, "Tone map effect ", "Shaders/Compiled/ToneMapping.vert.spv",
                                                     "Shaders/Compiled/ToneMapping.frag.spv", descLayoutCache,
                                                     EShaderBindingGroup::ToneMap);
    toneMappingPass->SetDisableDepthTest()
        .DisableStencil()
        .SetCullNone()
        .SetNullVertexBinding()
        //.SetColourOutputFormat(vk::Format::eR8G8B8A8Srgb)
        .SetPiplineNoMultiSampling();

    effects[EEffectType::ToneMappingPass] = std::move(toneMappingPass);

    //================================================================================
    auto fog = std::make_shared<VulkanUtils::VRasterEffect>(device, "Fog volume post processing", "Shaders/Compiled/FogVolume.vert.spv",
                                                            "Shaders/Compiled/FogVolume.frag.spv", descLayoutCache,
                                                            EShaderBindingGroup::FogBinding);

    fog->SetDisableDepthTest().SetNullVertexBinding().SetCullNone().SetPiplineNoMultiSampling();

    effects[EEffectType::FogVolume] = std::move(fog);

    //================================================================================

    auto lensFlare = std::make_shared<VulkanUtils::VRasterEffect>(device, "Lens flare post processing effect",
                                                                  "Shaders/Compiled/LensFlare.vert.spv",
                                                                  "Shaders/Compiled/LensFlare.frag.spv", descLayoutCache,
                                                                  EShaderBindingGroup::LensFlareBinding);

    lensFlare->SetDisableDepthTest().SetNullVertexBinding().SetCullNone().SetPiplineNoMultiSampling();

    effects[EEffectType::LensFlare] = std::move(lensFlare);

    //================================================================================
    auto luminanceHistrogram =
        std::make_shared<VulkanUtils::VComputeEffect>(device, "Luminance histogram", "Shaders/Compiled/LuminanceHistrogram.spv",
                                                      descLayoutCache, EShaderBindingGroup::LuminanceHistrogram);

    effects[EEffectType::LuminanceHistrogram] = std::move(luminanceHistrogram);

    //================================================================================
    auto averageLuminance =
        std::make_shared<VulkanUtils::VComputeEffect>(device, "Luminance average", "Shaders/Compiled/AverageLuminance.spv",
                                                      descLayoutCache, EShaderBindingGroup::AverageLuminance);

    effects[EEffectType::AverageLuminance] = std::move(averageLuminance);
    //================================================================================

    auto bilateralFilter =
        std::make_shared<VulkanUtils::VComputeEffect>(device, "Bilateral filter", "Shaders/Compiled/Bilaterial-Filter.spv",
                                                      descLayoutCache, EShaderBindingGroup::ComputePostProecess);

    effects[EEffectType::BilateralFilter] = std::move(bilateralFilter);

    BuildAllEffects();
}

std::shared_ptr<VulkanUtils::VEffect> EffectsLibrary::GetEffect(EEffectType type)
{
    assert(effects.contains(type));
    return effects[type];
}

void EffectsLibrary::BuildAllEffects()
{
    std::cout << "======== Effect reflections =========\n";
    for(auto& effect : effects)
    {
        std::cout << "------------------ |" << effect.second->GetName() << " | ------------------------\n";
        effect.second->GetReflectionData()->Print();
        effect.second->BuildEffect();
    }
}

void EffectsLibrary::Destroy()
{
    for(auto& effect : effects)
    {
        effect.second->Destroy();
    }
}
void EffectsLibrary::UpdatePerFrameWrites(const Renderer::ForwardRenderer&          sceneRenderer,
                                          VulkanUtils::VRayTracingDataManager&      rayTracingDataManager,
                                          VulkanUtils::RenderContext*               renderingContext,
                                          VulkanStructs::PostProcessingContext&     postProcessingContext,
                                          const VulkanUtils::VUniformBufferManager& uniformBufferManager)
{
    // TODO: fog binding is missing
}

void EffectsLibrary::ConfigureDescriptorWrites(const Renderer::ForwardRenderer&     sceneRenderer,
                                               VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                                               VulkanUtils::VRayTracingDataManager& rayTracingDataManager)
{
    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {

        // =========================
        // ComputePostProcess
        if(auto it = effects.find(EEffectType::ComputePostProcess); it != effects.end())
        {
            auto& e = it->second;
            // Example placeholder if you later want to write images/buffers
            // e->SetNumWrites(0, 2, 0);
            // e->WriteImage(i, 0, 0, sceneRenderer.GetForwardRendererResult());

            e->ApplyWrites(i);
        }
    }
}


}  // namespace ApplicationCore