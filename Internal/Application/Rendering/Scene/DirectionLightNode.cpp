//
// Created by wpsimon09 on 26/01/25.
//

#include "DirectionLightNode.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore {
    DirectionLightNode::DirectionLightNode(std::shared_ptr<StaticMesh> mesh): SceneNode(mesh), m_lightStruct()
    {
        m_sceneNodeMetaData.nodeType = ENodeType::DirectionalLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;
    }

    void DirectionLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void DirectionLightNode::Update() const
    {
        if (m_transformation->HasChanged())
        {
            m_lightStruct.direction = glm::vec3(m_transformation->GetRotationMatrix() * glm::vec4(glm::vec3(0.0f, 0.0f, -1.0f),0.0f));
            Utils::Logger::LogInfoClient("Directional light is pointing at " + std::to_string(m_lightStruct.direction.x) + std::to_string(m_lightStruct.direction.y) + std::to_string(m_lightStruct.direction.z));
        }
        SceneNode::Update();

    }
} // ApplicationCore