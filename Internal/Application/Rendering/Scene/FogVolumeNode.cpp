//
// Created by wpsimon09 on 23/06/25.
//

#include "FogVolumeNode.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {
FogVolumeNode::FogVolumeNode(): SceneNode() {
    m_sceneNodeMetaData.nodeType = ENodeType::FogVolume;
    m_sceneNodeMetaData.FrustumCull = false;
    m_sceneNodeMetaData.IsVolumeNode = true;
}

void FogVolumeNode::Update(SceneUpdateFlags& sceneUpdateFlags)
{
    SceneNode::Update(sceneUpdateFlags);
}

void FogVolumeNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{
    VulkanStructs::VDrawCallData drawCall;
    drawCall.postProcessingEffect = true;
    drawCall.effect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::FogVolume);

    renderingContext->AddDrawCall(drawCall);
}
} // ApplicationCore