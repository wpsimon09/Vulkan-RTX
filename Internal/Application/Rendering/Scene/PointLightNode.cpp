//
// Created by wpsimon09 on 04/02/25.
//

#include "PointLightNode.hpp"

namespace ApplicationCore {
    PointLightNode::PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh):LightNode<LightStructs::PointLight>(mesh),  m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::PointLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bOpaquePass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;
        m_sceneNodeMetaData.RenderingMetaData.bDebugGeometryPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bTransparentPass = false;

        m_index = sceneLightInfo.AddPointLight(&m_lightStruct);
    }

    PointLightNode::PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo,
        std::shared_ptr<StaticMesh> mesh, LightStructs::PointLight* pointLightData):LightNode::LightNode<LightStructs::PointLight>(mesh, pointLightData), m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::PointLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bOpaquePass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;
        m_sceneNodeMetaData.RenderingMetaData.bDebugGeometryPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bTransparentPass = false;


        m_index = sceneLightInfo.AddPointLight(&m_lightStruct);
        m_transformation->SetPosition(m_lightStruct.position);
    }

    void PointLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void PointLightNode::Update()
    {
        m_lightStruct.position = m_transformation->GetPosition();
        SceneNode::Update();
    }

    void PointLightNode::ProcessNodeRemove()
    {
        m_lightStruct.Reset();
        SceneNode::ProcessNodeRemove();
    }
} // ApplicationCore