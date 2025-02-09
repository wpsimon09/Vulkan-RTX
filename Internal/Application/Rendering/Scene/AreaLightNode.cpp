//
// Created by wpsimon09 on 09/02/25.
//

#include "AreaLightNode.hpp"

namespace ApplicationCore {
    AreaLightNode::AreaLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh): LightNode<LightStructs::AreaLight>(mesh), m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::AreaLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;

        m_index = sceneLightInfo.AddAreaLight(&m_lightStruct);
    }

    void AreaLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void AreaLightNode::Update()
    {
        LightNode<LightStructs::AreaLight>::Update();
        for (auto& edge : m_lightStruct.edges)
        {
            edge =  m_transformation->GetModelMatrix() * edge;
        }
    }

    void AreaLightNode::ProcessNodeRemove()
    {
        m_lightStruct.Reset();
    }
} // ApplicationCore