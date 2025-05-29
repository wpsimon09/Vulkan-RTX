//
// Created by wpsimon09 on 20/03/25.
//

#include "EffectsLibrary.hpp"

#include <memory>

#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"

namespace ApplicationCore {
EffectsLibrary::EffectsLibrary(const VulkanCore::VDevice&          device,
                               VulkanUtils::VUniformBufferManager& uniformBufferManager,
                               VulkanUtils::VRayTracingDataManager& rtxDataManager,
                               VulkanCore::VDescriptorLayoutCache& descLayoutCache)
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

    auto debugLine =
        std::make_shared<VulkanUtils::VRasterEffect>(device, "Debug lines", "Shaders/Compiled/DebugLines.vert.spv",
                                                     "Shaders/Compiled/DebugLines.frag.spv", descLayoutCache, ForwardUnlit);

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
                                                                EShaderBindingGroup::Debug);

    outline
        //->SetC()
        ->SetStencilTestOutline()
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetDepthOpAllways();


    effects[EEffectType::Outline] = std::move(outline);

    //===============================================================================

    auto debugShapes = std::make_shared<VulkanUtils::VRasterEffect>(device, "Debug shapes", "Shaders/Compiled/DebugLines.vert.spv",
                                                                    "Shaders/Compiled/DebugGeometry.frag.spv",
                                                                    descLayoutCache, EShaderBindingGroup::Debug);

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
    auto depthPrePass = std::make_shared<VulkanUtils::VRasterEffect>(device, "Depth-PrePass effect",
                                                                        "Shaders/Compiled/DepthPrePass.vert.spv",
                                                                        "Shaders/Compiled/DepthPrePass.frag.spv",
                                                                        descLayoutCache, EShaderBindingGroup::ForwardUnlitNoMaterial);
    depthPrePass->SetVertexInputMode(EVertexInput::PositionOnly).SetDepthOpLess();


    effects[EEffectType::DepthPrePass] = std::move(depthPrePass);

    //===============================================================================
    VulkanCore::RTX::RTXShaderPaths rtxShaderPaths;
    rtxShaderPaths.rayGenPath     = "Shaders/Compiled/SimpleRayTracing.rgen.spv";
    rtxShaderPaths.missPath       = "Shaders/Compiled/SimpleRayTracing.miss.spv";
    rtxShaderPaths.missShadowPath = "Shaders/Compiled/SimpleRayTracing.miss2.spv";
    rtxShaderPaths.rayHitPath     = "Shaders/Compiled/SimpleRayTracing.chit.spv";
    //auto rayTracingEffect =
      //  std::make_shared<VulkanUtils::VRayTracingEffect>(device, rtxShaderPaths, "Ray tracing ", m_descLayoutCache);

    //effects[EEffectType::RayTracing] = std::move(rayTracingEffect);



    BuildAllEffects();
    ConfigureDescriptorWrites(uniformBufferManager);
}

std::shared_ptr<VulkanUtils::VEffect> EffectsLibrary::GetEffect(EEffectType type)
{
    assert(effects.contains(type));
    return effects[type];
}

void EffectsLibrary::BuildAllEffects()
{
    std::cout<<"======== Effect reflections =========\n\n\n";
    for(auto& effect : effects)
    {
        std::cout<<effect.second->GetName() <<" | ------------------------\n";
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
void EffectsLibrary::UpdatePerFrameWrites(VulkanUtils::RenderContext*       renderingContext,
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

                    e->SetNumWrites(0, 1200, 0);
                    e->WriteImageArray(i, 0, 4, uniformBufferManager.GetAll2DTextureDescriptorImageInfo());
                    break;
                }
                case EShaderBindingGroup::RayTracing:{

                    e->SetNumWrites(4, 4, 0);

                    break;
                }
                case EShaderBindingGroup::ForwardUnlit:{
                    e->SetNumWrites(4, 4, 0);
                    //===================================

                }
                case EShaderBindingGroup::ForwardUnlitNoMaterial:{
                    break;
                }

                case EShaderBindingGroup::Skybox: {
                    //====================================
                    // global data
                    e->SetNumWrites(1, 0);
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);
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

void EffectsLibrary::ConfigureDescriptorWrites(VulkanUtils::VUniformBufferManager& uniformBufferManager)
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
                case EShaderBindingGroup::Debug: {

                    e->SetNumWrites(2, 0, 0);

                    //========================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //========================
                    // per model data
                    e->WriteBuffer(i, 1, 0, uniformBufferManager.GetPerObjectBuffer(i));

                    break;
                }

                case EShaderBindingGroup::ForwardLit: {

                    e->SetNumWrites(7, 4, 0);
                    //===================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //===================================
                    // materials
                    e->WriteBuffer(i, 0, 1, uniformBufferManager.GetMaterialDescriptionBuffer(i));

                    //===================================
                    // lighting information
                    e->WriteBuffer(i, 0, 2, uniformBufferManager.GetLightBufferDescriptorInfo()[i]);

                    //===================================
                    // std::vector<PerObjectData> SSBO.
                    e->WriteBuffer(i, 1, 0, uniformBufferManager.GetPerObjectBuffer(i));

                    break;
                }
                case EShaderBindingGroup::RayTracing:{


                    break;
                }
            case EShaderBindingGroup::ForwardUnlitNoMaterial:{
                    e->SetNumWrites(2, 0, 0);

                    //===================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //===================================
                    // std::vector<PerObjectData> SSBO.
                    e->WriteBuffer(i, 1, 0, uniformBufferManager.GetPerObjectBuffer(i));

                    break;
                }
                case EShaderBindingGroup::ForwardUnlit:{
                    e->SetNumWrites(7, 4, 0);
                    //===================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);

                    //===================================
                    // materials
                    e->WriteBuffer(i, 0, 1, uniformBufferManager.GetMaterialDescriptionBuffer(i));

                    //===================================
                    // std::vector<PerObjectData> SSBO.
                    e->WriteBuffer(i, 1, 0, uniformBufferManager.GetPerObjectBuffer(i));

                }

                case EShaderBindingGroup::Skybox: {
                    e->SetNumWrites(1, 0);
                    //====================================
                    // global data
                    e->WriteBuffer(i, 0, 0, uniformBufferManager.GetGlobalBufferDescriptorInfo()[i]);
                    break;
                }

                default: {
                    throw std::runtime_error("Unsupported bindinggroup !");
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