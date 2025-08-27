//
// Created by wpsimon09 on 23/06/25.
//

#include "FogVolumeNode.hpp"

#include "Scene.hpp"
#include "Application/ApplicationState/ApplicationState.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {
FogVolumeNode::FogVolumeNode(std::shared_ptr<ApplicationCore::StaticMesh> mesh)
    : SceneNode(mesh)
{
    m_sceneNodeMetaData.nodeType     = ENodeType::FogVolume;
    m_sceneNodeMetaData.FrustumCull  = false;
    m_sceneNodeMetaData.IsVolumeNode = true;
}

void FogVolumeNode::Update(SceneUpdateContext& sceneUpdateFlags)
{
    sceneUpdateFlags.applicationState->pSetFogVolumeParameters(&m_parameters);
    SceneNode::Update(sceneUpdateFlags);
}

void FogVolumeNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{
    VulkanStructs::VDrawCallData drawCall;
    drawCall.postProcessingEffect = true;
    drawCall.vertexData           = m_mesh->GetMeshData()->vertexData;
    drawCall.indexData            = m_mesh->GetMeshData()->indexData;
    drawCall.effect               = static_cast<uint32_t>(EEffectType::FogVolume);
    drawCall.material             = nullptr;
    if(m_sceneNodeMetaData.IsVisible)
    {
        renderingContext->AddDrawCall(drawCall);
    }
}
FogVolumeParameters& FogVolumeNode::GetParameters()
{
    return m_parameters;
}

void FogVolumeNode::ProcessNodeRemove()
{
    SceneNode::ProcessNodeRemove();
}
void FogVolumeNode::ProcessNodeRemove(SceneData& sceneData) {

}

void FogVolumeNode::ProcessNodeRemove(const SceneNode& node, SceneData& sceneData)
{
    //SceneNode::ProcessNodeRemove(node, sceneData);
}
}  // namespace ApplicationCore