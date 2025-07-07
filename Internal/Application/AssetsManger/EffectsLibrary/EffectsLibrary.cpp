//
// Created by wpsimon09 on 20/03/25.
//

#include "EffectsLibrary.hpp"

#include "Application/Utils/LookUpTables.hpp"

#include <memory>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/ForwardRenderer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

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
                                                     EShaderBindingGroup::ForwardUnlitNoMaterial);
    depthPrePass->SetVertexInputMode(EVertexInput::PositionOnly).SetDepthOpLess();


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
        .SetColourOutputFormat(vk::Format::eR16G16Sfloat)
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
                                                                  "Shaders/Compiled/LensFlare.frag.spv",
                                                                  descLayoutCache, EShaderBindingGroup::LensFlareBinding);

    lensFlare->SetDisableDepthTest().SetNullVertexBinding().SetCullNone().SetPiplineNoMultiSampling();

    effects[EEffectType::LensFlare] = std::move(lensFlare);

    //================================================================================

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
    for(auto& effect : effects)
    {
        auto& e = effect.second;

        //=========================
        // for each frame in flight
        for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {

            switch(e->GetBindingGroup())
            {

                case EShaderBindingGroup::ForwardLit: {

                    e->SetNumWrites(0, 4200, 0);
                    e->WriteImageArray(i, 1, 1, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());

                    if(renderingContext->irradianceMap)
                    {
                        e->WriteImage(i, 1, 2, renderingContext->irradianceMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));
                    }
                    if(renderingContext->prefilterMap)
                    {
                        e->WriteImage(i, 1, 3, renderingContext->prefilterMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));
                    }
                    if(renderingContext->irradianceMap)
                    {
                        e->WriteImage(i, 1, 4, renderingContext->brdfMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                    }
                    break;
                }
                case EShaderBindingGroup::RayTracing: {

                    break;
                }
                case EShaderBindingGroup::ForwardUnlit: {
                    e->SetNumWrites(0, 3200, 0);
                    e->WriteImageArray(i, 1, 0, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());
                    break;
                }
                case EShaderBindingGroup::ForwardUnlitNoMaterial: {
                    break;
                }
                case EShaderBindingGroup::ToneMap: {
                    e->SetNumWrites(0, 1, 0);
                    if(postProcessingContext.sceneRender != nullptr)
                    {
                        e->WriteImage(i, 1, 0, postProcessingContext.sceneRender->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                    }
                    break;
                }

                case EShaderBindingGroup::Skybox: {
                    //====================================
                    // global data
                    e->SetNumWrites(0, 1);
                    if(renderingContext->hdrCubeMap)
                    {
                        e->WriteImage(i, 1, 0, renderingContext->hdrCubeMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                    }
                    break;
                }
                case EShaderBindingGroup::ShadowRT: {
                    e->SetNumWrites(0, 0, 1);
                    e->WriteAccelerationStrucutre(i, 0, 2, rayTracingDataManager.GetTLAS());
                    break;
                }
                case EShaderBindingGroup::FogBinding: {
                    e->SetNumWrites(0, 0, 1);
                    e->WriteAccelerationStrucutre(i, 0, 6, rayTracingDataManager.GetTLAS());
                    break;
                }

                default: {
                    // throw std::runtime_error("Unsupported bindinggroup !");
                    break;
                }
            }

            //===================================
            // apply writes
            e->ApplyWrites(i);
        }
    }
}

void EffectsLibrary::ConfigureDescriptorWrites(const Renderer::ForwardRenderer&     sceneRenderer,
                                               VulkanUtils::VUniformBufferManager&  uniformBufferManager,
                                               VulkanUtils::VRayTracingDataManager& rayTracingDataManager)
{
    for(auto& effect : effects)
    {
        auto& e = effect.second;

        //=========================
        // for each frame in flight
        for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {

            switch(e->GetBindingGroup())
            {
                case EShaderBindingGroup::ForwardUnlitNoMaterial: {

                    e->SetNumWrites(3, 0, 1);

                    //========================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //========================
                    // per model data
                    e->WriteBuffer(i, 0, 1, uniformBufferManager.GetPerObjectBuffer(i));

                    break;
                }

                case EShaderBindingGroup::ForwardLit: {

                    e->SetNumWrites(7, 5, 0);
                    //===================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //===================================
                    // materials
                    e->WriteBuffer(i, 0, 1, uniformBufferManager.GetPerObjectBuffer(i));

                    //===================================
                    // lighting information
                    e->WriteBuffer(i, 0, 2, uniformBufferManager.GetMaterialDescriptionBuffer(i));

                    //===================================
                    // std::vector<PerObjectData> SSBO.
                    e->WriteBuffer(i, 0, 3, uniformBufferManager.GetLightBufferDescriptorInfo()[i]);

                    //===================================
                    // for ray query we need acceleration strucutre
                    e->WriteImage(i, 0, 4,
                                  sceneRenderer.GetShadowMapOutput().GetPrimaryImage().GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::Sampler2D));

                    break;
                }
                case EShaderBindingGroup::RayTracing: {
                    break;
                }
                case EShaderBindingGroup::ForwardUnlit: {
                    e->SetNumWrites(7, 4, 0);
                    //===================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //===================================
                    // materials
                    e->WriteBuffer(i, 0, 1, uniformBufferManager.GetPerObjectBuffer(i));

                    //===================================
                    // std::vector<PerObjectData> SSBO.
                    e->WriteBuffer(i, 0, 2, uniformBufferManager.GetMaterialDescriptionBuffer(i));
                    break;
                }
                case EShaderBindingGroup::ShadowRT: {
                    e->SetNumWrites(3, 4, 1);

                    //==================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //===================================
                    // ligth buffer
                    e->WriteBuffer(i, 0, 1, uniformBufferManager.GetLightBufferDescriptorInfo()[i]);

                    //===================================
                    // acceleration structure for shadows
                    e->WriteAccelerationStrucutre(i, 0, 2, rayTracingDataManager.GetTLAS());

                    // depth texture is written in scene renderer constructror

                    //===================================
                    // blue noise texture
                    e->WriteImage(i, 0, 4,
                                  MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::Sampler2D));

                    break;
                }

                case EShaderBindingGroup::Skybox: {
                    e->SetNumWrites(1, 0);

                    //====================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);
                    break;
                }
                case EShaderBindingGroup::ToneMap: {
                    e->SetNumWrites(1, 2, 0);

                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetPostProcessingBufferDescriptorInfo(i));
                    //e->WriteImage(i, 0, 1, sceneRenderer.GetRenderedImageConst(i));
                    break;
                }
                case EShaderBindingGroup::FogBinding: {
                    e->SetNumWrites(4, 7, 2);

                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    e->WriteImage(i, 0, 1,
                                  sceneRenderer.GetShadowMapOutput().GetPrimaryImage().GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::Sampler2D));
                    e->WriteImage(i, 0, 2,
                                  sceneRenderer.GetPositionBufferOutput().GetResolvedImage().GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::SamplerDepth));
                    e->WriteImage(i, 0, 3,
                                  MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::Sampler2D));
                    e->WriteImage(i, 0, 4,
                                  sceneRenderer.GetLightPassOutput().GetResolvedImage().GetDescriptorImageInfo(
                                      VulkanCore::VSamplers::Sampler2D));

                    e->WriteBuffer(i, 0, 5, uniformBufferManager.GetLightBufferDescriptorInfo()[i]);

                    e->WriteAccelerationStrucutre(i, 0, 6, rayTracingDataManager.GetTLAS());

                    e->WriteBuffer(i, 1, 0, uniformBufferManager.GetFogVolumParametersBufferDescriptorInfo(i));
                    break;
                }
                case EShaderBindingGroup::LensFlareBinding:{
                    e->SetNumWrites(2, 2, 0);

                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    e->WriteImage(i, 0, 1,
                MathUtils::LookUpTables.BlueNoise1024->GetHandle()->GetDescriptorImageInfo(
                    VulkanCore::VSamplers::Sampler2D));

                    // binding 2 is writteing in post processing renderer

                    e->WriteBuffer(i, 0, 3,uniformBufferManager.GetLightBufferDescriptorInfo()[i]);

                    break;
                }

                default: {
                    throw std::runtime_error("Unsupported binding group !");
                    break;
                }
            }

            //===================================
            // apply writes
            e->ApplyWrites(i);
        }
    }
}


}  // namespace ApplicationCore