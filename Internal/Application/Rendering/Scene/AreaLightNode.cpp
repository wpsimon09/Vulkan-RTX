//
// Created by wpsimon09 on 09/02/25.
//

#include "AreaLightNode.hpp"

#include "Application/Structs/LightStructs.hpp"

namespace ApplicationCore {
    AreaLightNode::AreaLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh, LightStructs::AreaLight* areaLightData): LightNode<LightStructs::AreaLight>(mesh, areaLightData), m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::AreaLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bOpaquePass = false;
        m_sceneNodeMetaData.RenderingMetaData.bRTXPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;
        m_sceneNodeMetaData.RenderingMetaData.bDebugGeometryPass = true;

        m_index = sceneLightInfo.AddAreaLight(&m_lightStruct);
        m_transformation->SetPosition(m_lightStruct.position);
        m_transformation->SetScale(glm::vec3(m_lightStruct.scale, 1.0f));
        m_transformation->SetRotation(m_lightStruct.orientation);

    }

    void AreaLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void AreaLightNode::Update()
    {
        LightNode<LightStructs::AreaLight>::Update();
        int i = 0;
        m_lightStruct.position = m_transformation->GetPosition();
        m_lightStruct.scale.x = m_transformation->GetScale().x;
        m_lightStruct.scale.y = m_transformation->GetScale().y;
        m_lightStruct.orientation = m_transformation->GetRotationsQuat();
        for (auto& edge : m_lightStruct.GetAreaLightEdges())
        {
            auto newEdge =  m_transformation->GetModelMatrix() * edge;
            m_lightStruct.edges[i] = newEdge;
            i++;
        }
    }

    void AreaLightNode::ProcessNodeRemove()
    {
        m_lightStruct.Reset();
    
    }
} // ApplicationCore