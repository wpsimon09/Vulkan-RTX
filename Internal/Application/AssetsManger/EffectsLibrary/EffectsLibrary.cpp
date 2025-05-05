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
#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"

namespace ApplicationCore {
EffectsLibrary::EffectsLibrary(const VulkanCore::VDevice& device, VulkanUtils::VResourceGroupManager& pushDescriptorManager)
{
    auto frowardEffect = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Forward lit", "Shaders/Compiled/BasicTriangle.vert.spv", "Shaders/Compiled/GGXColourFragmentMultiLight.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::ForwardShading));

    frowardEffect->SetTopology(vk::PrimitiveTopology::eTriangleList);

    if(GlobalVariables::RenderingOptions::PreformDepthPrePass)
    {
        frowardEffect->SetDisableDepthWrite();
    }

    effects[EEffectType::ForwardShader] = std::move(frowardEffect);

    //==============================================================================

    auto transparentEffect = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Forward lit transparent", "Shaders/Compiled/BasicTriangle.vert.spv",
        "Shaders/Compiled/GGXColourFragmentMultiLight.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::ForwardShading));

    transparentEffect->SetTopology(vk::PrimitiveTopology::eTriangleList).EnableAdditiveBlending().SetDepthOpLessEqual();
    if(GlobalVariables::RenderingOptions::PreformDepthPrePass)
    {
        transparentEffect->SetDisableDepthWrite();
    }


    effects[EEffectType::AplhaBlend] = std::move(transparentEffect);

    //==============================================================================

    auto editorBillboards = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Editor billboards", "Shaders/Compiled/EditorBillboard.vert.spv", "Shaders/Compiled/EditorBilboard.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::UnlitSingleTexture));

    editorBillboards->SetTopology(vk::PrimitiveTopology::eTriangleList).SetCullNone().SetVertexInputMode(EVertexInput::Position_UV)
        //.SetDepthOpLessEqual()
        ;
    effects[EEffectType::EditorBilboard] = std::move(editorBillboards);

    //==============================================================================

    auto debugLine = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Debug lines", "Shaders/Compiled/DebugLines.vert.spv", "Shaders/Compiled/DebugLines.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::Basic));

    debugLine->SetTopology(vk::PrimitiveTopology::eTriangleList)
        .SetCullNone()
        .SetPolygonLine()
        .SetLineWidth(2)
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetDepthOpLessEqual();

    effects[EEffectType::DebugLine] = std::move(debugLine);

    //==============================================================================

    auto outline = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Outline", "Shaders/Compiled/DebugLines.vert.spv", "Shaders/Compiled/Outliines.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::Basic));

    outline
        //->SetC()
        ->SetStencilTestOutline()
        .SetVertexInputMode(EVertexInput::PositionOnly)
        .SetDepthOpAllways();

    effects[EEffectType::Outline] = std::move(outline);

    //===============================================================================

    auto debugShapes = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Debug shapes", "Shaders/Compiled/DebugLines.vert.spv", "Shaders/Compiled/DebugGeometry.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::Basic));

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

    auto skybox = std::make_shared<VulkanUtils::VRasterEffect>(
        device, "Sky Box", "Shaders/Compiled/SkyBox.vert.spv", "Shaders/Compiled/SkyBox.frag.spv",
        pushDescriptorManager.GetResourceGroup(VulkanUtils::EDescriptorLayoutStruct::UnlitSingleTexture));


    skybox->SetCullNone().SetVertexInputMode(EVertexInput::PositionOnly).SetDisableDepthWrite().SetDepthOpLessEqual().DisableStencil();


    effects[EEffectType::SkyBox] = std::move(skybox);

    BuildAllEffects();
}

std::shared_ptr<VulkanUtils::VEffect> EffectsLibrary::GetEffect(EEffectType type)
{
    assert(effects.contains(type));
    return effects[type];
}

void EffectsLibrary::BuildAllEffects()
{
    for(auto& effect : effects)
    {
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
}  // namespace ApplicationCore