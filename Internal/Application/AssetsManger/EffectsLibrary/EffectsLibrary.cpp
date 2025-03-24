//
// Created by wpsimon09 on 20/03/25.
//

#include "EffectsLibrary.hpp"

#include <memory>

#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace ApplicationCore {
    EffectsLibrary::EffectsLibrary(const VulkanCore::VDevice& device,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager)
    {
        auto frowardEffect = std::make_shared<VulkanUtils::VEffect>(
            device, "Forward lit",
            "Shaders/Compiled/BasicTriangle.vert.slang.spv",
            "Shaders/Compiled/GGXColourFragmentMultiLight.frag.slang.spv",
            pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::ForwardShading));

        frowardEffect->SetTopology(vk::PrimitiveTopology::eTriangleList);
        effects[EEffectType::ForwardShader] = std::move(frowardEffect);

        //==============================================================================

        auto transparentEffect = std::make_shared<VulkanUtils::VEffect>(
            device, "Forward lit transparent",
            "Shaders/Compiled/BasicTriangle.vert.slang.spv",
            "Shaders/Compiled/GGXColourFragmentMultiLight.frag.slang.spv",
            pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::ForwardShading));

        transparentEffect
            ->SetTopology(vk::PrimitiveTopology::eTriangleList)
            .EnableAdditiveBlending();

        effects[EEffectType::AplhaBlend] = std::move(transparentEffect);

        //==============================================================================

        auto editorBillboards = std::make_shared<VulkanUtils::VEffect>(
            device, "Editor billboards",
            "Shaders/Compiled/EditorBillboard.vert.slang.spv",
            "Shaders/Compiled/EditorBilboard.frag.slang.spv",
            pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::UnlitSingleTexture));

        editorBillboards
            ->SetTopology(vk::PrimitiveTopology::eTriangleList)
            .SetCullNone();
        effects[EEffectType::EditorBilboard] = std::move(editorBillboards);

        //==============================================================================

        auto debugLine = std::make_shared<VulkanUtils::VEffect>(
                   device, "Debug lines",
                   "Shaders/Compiled/BasicTriangle.vert.slang.spv",
                   "Shaders/Compiled/DebugLines.frag.slang.spv",
                   pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::Basic));

        debugLine
            ->SetTopology(vk::PrimitiveTopology::eTriangleList)
            .SetCullNone()
            .SetPolygonLine()
            .SetLineWidth(2);

        effects[EEffectType::DebugLine] = std::move(debugLine);

        //==============================================================================

        auto outline = std::make_shared<VulkanUtils::VEffect>(
                   device, "Outline",
                   "Shaders/Compiled/BasicTriangle.vert.slang.spv",
                   "Shaders/Compiled/Outliines.frag.slang.spv",
                   pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::Basic));

        outline
            ->SetCullFrontFace()
            .SetLineWidth(7)
            .SetPolygonLine()
            .SetDisableDepthWrite();

        effects[EEffectType::Outline] = std::move(outline);

        //===============================================================================

        auto debugShapes = std::make_shared<VulkanUtils::VEffect>(
            device, "Debug shapes",
            "Shaders/Compiled/BasicTriangle.vert.slang.spv",
            "Shaders/Compiled/DebugGeometry.frag.slang.spv",
                   pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::Basic));

        debugShapes->SetCullNone()
        .SetLineWidth(7)
        .SetPolygonLine()
        .SetTopology(vk::PrimitiveTopology::eLineList);

        effects[EEffectType::DebugLine] = std::move(debugShapes);

        //===============================================================================

        BuildAllEffects();

    }

    std::shared_ptr<VulkanUtils::VEffect> EffectsLibrary::GetEffect(EEffectType type)
    {
        assert(effects.contains(type));
        return effects[type];
    }

    void EffectsLibrary::BuildAllEffects()
    {
        for (auto& effect : effects)
        {
            effect.second->BuildEffect();
        }
    }

    void EffectsLibrary::Destroy()
    {
        for (auto& effect : effects)
        {
            effect.second->Destroy();
        }
    }
} // ApplicationCore