//
// Created by wpsimon09 on 28/09/2025.
//

#include "AtmosphereSceneNode.hpp"
#include "Application/Enums/ClientEnums.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {
AtmosphereSceneNode::AtmosphereSceneNode(std::shared_ptr<ApplicationCore::StaticMesh> mesh)
    : m_parameters()
{
    m_sceneNodeMetaData.nodeType = ENodeType::Atmosphere;
}

void AtmosphereSceneNode::Update(SceneUpdateContext& sceneUpdateFlags)
{
    SceneNode::Update(sceneUpdateFlags);
}
void AtmosphereSceneNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{
    auto drawCal = VulkanStructs::VDrawCallData();

    if(m_sceneNodeMetaData.IsVisible)
    {
        renderingContext->AddDrawCall(drawCal);
    }
    SceneNode::Render(effectsLibrary, renderingContext);
}
AtmosphereParameters& AtmosphereSceneNode::GetParameters() {}
void                  AtmosphereSceneNode::ProcessNodeRemove()
{
    SceneNode::ProcessNodeRemove();
}
void AtmosphereSceneNode::ProcessNodeRemove(SceneData& sceneData)
{
    SceneNode::ProcessNodeRemove(sceneData);
}
void AtmosphereSceneNode::ProcessNodeRemove(const SceneNode& node, SceneData& sceneData)
{
    SceneNode::ProcessNodeRemove(node, sceneData);
}
}  // namespace ApplicationCore