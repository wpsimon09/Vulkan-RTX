//
// Created by wpsimon09 on 04/02/25.
//

#include "PointLightNode.hpp"

namespace ApplicationCore {
    PointLightNode::PointLightNode(std::shared_ptr<StaticMesh> mesh):SceneNode(mesh), m_lightStruct()
    {
        m_sceneNodeMetaData.nodeType = ENodeType::PointLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;
    }

    void PointLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void PointLightNode::Update() const
    {
        m_lightStruct.position = m_transformation->GetPosition();
        SceneNode::Update();
    }
} // ApplicationCore