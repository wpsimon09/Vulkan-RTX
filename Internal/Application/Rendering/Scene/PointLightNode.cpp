//
// Created by wpsimon09 on 04/02/25.
//

#include "PointLightNode.hpp"

#include "Application/Rendering/Material/PBRMaterial.hpp"

namespace ApplicationCore {
PointLightNode::PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh)
    : LightNode<LightStructs::PointLight>(mesh)
    , m_sceneLightInfo(sceneLightInfo)
{
    m_sceneNodeMetaData.nodeType = ENodeType::PointLightNode;

    m_index = sceneLightInfo.AddPointLight(&m_lightStruct);
}

PointLightNode::PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo,
                               std::shared_ptr<StaticMesh>   mesh,
                               LightStructs::PointLight*     pointLightData)
    : LightNode::LightNode<LightStructs::PointLight>(mesh, pointLightData)
    , m_sceneLightInfo(sceneLightInfo)
{
    m_sceneNodeMetaData.nodeType = ENodeType::PointLightNode;


    m_index = sceneLightInfo.AddPointLight(&m_lightStruct);
    m_transformation->SetPosition(m_lightStruct.position);
}

void PointLightNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{
    if(!renderingContext->RenderBillboards)
        return;
    if(m_mesh && m_sceneNodeMetaData.IsVisible)
    {

        // frustrum culling
        if(m_sceneNodeMetaData.FrustumCull && GlobalVariables::RenderingOptions::EnableFrustrumCulling)
        {
            if(!VulkanUtils::IsInViewFrustum(&m_mesh->GetMeshData()->bounds, m_transformation->GetModelMatrix(),
                                             renderingContext->view, renderingContext->projection))
            {
                return;
            }
        }

        VulkanStructs::DrawCallData data;
        data.firstIndex = 1;

        data.indexCount = m_mesh->GetMeshIndexCount();
        // data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);

        data.bounds     = &m_mesh->GetMeshData()->bounds;
        data.vertexData = &m_mesh->GetMeshData()->vertexData;
        data.indexData  = &m_mesh->GetMeshData()->indexData;

        data.modelMatrix = m_transformation->GetModelMatrix();
        data.material    = m_mesh->GetMaterial().get();
        if(renderingContext->WireFrameRendering)
            data.effect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::DebugLine);
        else
        {
            data.effect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::EditorBilboard);
        }

        data.position = m_transformation->GetPosition();

        data.bounds = &m_mesh->GetMeshData()->bounds;

        renderingContext->AddDrawCall(data);

        if(m_sceneNodeMetaData.IsSelected)
        {
            data.effect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::Outline);
            renderingContext->AddDrawCall(data);
        }
    }
}

void PointLightNode::Update(bool& needsUpdate)
{
    m_lightStruct.position          = m_transformation->GetPosition();
    m_lightStruct.isPointLightInUse = m_sceneNodeMetaData.IsVisible;
    SceneNode::Update(needsUpdate);
}

void PointLightNode::ProcessNodeRemove()
{
    m_lightStruct.Reset();
    SceneNode::ProcessNodeRemove();
}
}  // namespace ApplicationCore