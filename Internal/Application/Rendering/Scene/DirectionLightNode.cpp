//
// Created by wpsimon09 on 26/01/25.
//

#include "DirectionLightNode.hpp"

#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore {
    DirectionLightNode::DirectionLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh,LightStructs::DirectionalLight* directionalLightData): LightNode<LightStructs::DirectionalLight>(mesh, directionalLightData), m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::DirectionalLightNode;
        sceneLightInfo.DirectionalLightInfo = &m_lightStruct;
        m_transformation->SetPosition(m_lightStruct.direction);
    }


    void DirectionLightNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
    {
        SceneNode::Render(effectsLibrary, renderingContext);
    }

    void DirectionLightNode::Update()
    {
        m_lightStruct.direction = glm::vec3(m_transformation->GetRotationMatrix() * glm::vec4(glm::vec3(0.0f, 0.0f, -1.0f),0.0f));;
        SceneNode::Update();
    }

    void DirectionLightNode::ProcessNodeRemove()
    {
        SceneNode::ProcessNodeRemove();
        m_lightStruct.Reset();
        m_sceneLightInfo.DirectionalLightInfo = nullptr;
    }
} // ApplicationCore