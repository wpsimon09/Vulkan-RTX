//
// Created by wpsimon09 on 09/02/25.
//

#include "AreaLightNode.hpp"

#include "Application/Structs/LightStructs.hpp"

namespace ApplicationCore {
    AreaLightNode::AreaLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh): LightNode<LightStructs::AreaLight>(mesh), m_sceneLightInfo(sceneLightInfo)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::AreaLightNode;
        m_sceneNodeMetaData.RenderingMetaData.bMainLightPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bRTXPass = false;
        m_sceneNodeMetaData.RenderingMetaData.bEditorBillboardPass = true;
        m_sceneNodeMetaData.RenderingMetaData.bDebugGeometryPass = true;

        m_index = sceneLightInfo.AddAreaLight(&m_lightStruct);
    }

    void AreaLightNode::Render(VulkanStructs::RenderContext* renderingContext) const
    {
        SceneNode::Render(renderingContext);
    }

    void AreaLightNode::Update()
    {
        LightNode<LightStructs::AreaLight>::Update();
        int i = 0;
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