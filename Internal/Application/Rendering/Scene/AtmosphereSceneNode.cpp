//
// Created by wpsimon09 on 28/09/2025.
//

#include "AtmosphereSceneNode.hpp"
#include "Application/Enums/ClientEnums.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Application/ApplicationState/ApplicationState.hpp"
#include <vulkan/vulkan.hpp>

namespace ApplicationCore {
AtmosphereSceneNode::AtmosphereSceneNode(std::shared_ptr<ApplicationCore::StaticMesh> mesh)
    : m_parameters()
    , m_defaultParmas{}
    , m_drawCall{}
{
    m_sceneNodeMetaData.nodeType     = ENodeType::Atmosphere;
    m_sceneNodeMetaData.IsVolumeNode = true;
}

void AtmosphereSceneNode::Update(SceneUpdateContext& sceneUpdateFlags)
{
    sceneUpdateFlags.applicationState->pSetAtmosphereParameters(&m_parameters);

    SceneNode::Update(sceneUpdateFlags);
}

void AtmosphereSceneNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{

    if(m_sceneNodeMetaData.IsVisible)
    {
        renderingContext->atmosphereCall = VulkanStructs::VDrawCallData{};
    }

    // SceneNode::Render(effectsLibrary, renderingContext);
}
AtmosphereParameters& AtmosphereSceneNode::GetParameters()
{
    return m_parameters;
}

void AtmosphereSceneNode::ProcessNodeRemove()
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
const AtmosphereParameters& AtmosphereSceneNode::GetDefaultAtmosphereParams()
{
    return m_defaultParmas;
}
}  // namespace ApplicationCore