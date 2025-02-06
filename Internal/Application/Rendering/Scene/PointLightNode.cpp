//
// Created by wpsimon09 on 04/02/25.
//

#include "PointLightNode.hpp"

namespace ApplicationCore {
    PointLightNode::PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh):LightNode<LightStructs::PointLight>(mesh),  m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::PointLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;

        m_index = sceneLightInfo.AddPointLight(&m_lightStruct);
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

    void PointLightNode::ProcessNodeRemove()
    {
        SceneNode::ProcessNodeRemove();
        m_sceneLightInfo.PointLightInfos[m_index]->colour = {0.0f, 0.0f, 0.0f,0.f};
        m_sceneLightInfo.PointLightInfos[m_index]->position = {0.0f, 0.0f, 0.0f};
    }
} // ApplicationCore