//
// Created by wpsimon09 on 20/03/25.
//

#include "EffectsLibrary.hpp"

#include <memory>

#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace ApplicationCore {
    EffectsLibrary::EffectsLibrary(const VulkanCore::VDevice& device,  Renderer::RenderingSystem& renderingSystem,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager)
    {
        auto frowardEffect = std::make_shared<VulkanUtils::VEffect>(
            device, "Shaders/Compiled/BasicTriangle.vert.slang.spv",
            "Shaders/Compiled/GGXColourFragmentMultiLight.frag.slang.spv",
            renderingSystem.GetSceneRenderer().GetRenderTarget(),
            pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::ForwardShading));

        frowardEffect->SetTopology(vk::PrimitiveTopology::eTriangleList);
        effects[EEffectType::ForwardShader] = std::move(frowardEffect);

        //==============================================================================

        auto transparentEffect = std::make_shared<VulkanUtils::VEffect>(
            device, "Shaders/Compiled/BasicTriangle.vert.slang.spv",
            "Shaders/Compiled/GGXColourFragmentMultiLight.frag.slang.spv",
            renderingSystem.GetSceneRenderer().GetRenderTarget(),
            pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::ForwardShading));

        transparentEffect
            ->SetTopology(vk::PrimitiveTopology::eTriangleList)
            .EnableAdditiveBlending();

        effects[EEffectType::AplhaBlend] = std::move(transparentEffect);

        //==============================================================================

        auto editorBillboards = std::make_shared<VulkanUtils::VEffect>(
            device, "Shaders/Compiled/EditorBillboard.vert.slang.spv",
            "Shaders/Compiled/EditorBilboard.frag.slang.spv",
            renderingSystem.GetSceneRenderer().GetRenderTarget(),
            pushDescriptorManager.GetPushDescriptor(VulkanUtils::EDescriptorLayoutStruct::UnlitSingleTexture));

        editorBillboards
            ->SetTopology(vk::PrimitiveTopology::eTriangleList)
            .SetCullNone();


    }
} // ApplicationCore