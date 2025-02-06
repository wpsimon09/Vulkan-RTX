//
// Created by wpsimon09 on 26/01/25.
//

#include "DirectionLightNode.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore {
    DirectionLightNode::DirectionLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh): SceneNode(mesh), m_lightStruct()
    {
        m_sceneNodeMetaData.nodeType = ENodeType::DirectionalLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;

        sceneLightInfo.DirectionalLightInfo = &m_lightStruct;
    }

    void DirectionLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void DirectionLightNode::Update() const
    {
        m_lightStruct.direction = glm::vec3(m_transformation->GetRotationMatrix() * glm::vec4(glm::vec3(0.0f, 0.0f, -1.0f),0.0f));;
        SceneNode::Update();
    }

    void DirectionLightNode::ProcessNodeRemove()
    {
        SceneNode::ProcessNodeRemove();
        m_lightStruct.direction = glm::vec3(0.0f, 0.0f, -1.0f);
        m_lightStruct.colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
} // ApplicationCore