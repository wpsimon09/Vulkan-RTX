//
// Created by wpsimon09 on 09/02/25.
//

#include "AreaLightNode.hpp"

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {
AreaLightNode::AreaLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh, LightStructs::AreaLight* areaLightData)
    : LightNode<LightStructs::AreaLight>(mesh, areaLightData)
    , m_sceneLightInfo(sceneLightInfo)
{
    m_sceneNodeMetaData.nodeType = ENodeType::AreaLightNode;

    m_index = sceneLightInfo.AddAreaLight(&m_lightStruct);
    m_transformation->SetPosition(m_lightStruct.position);
    m_transformation->SetScale(glm::vec3(m_lightStruct.scale, 1.0f));
    m_transformation->SetRotation(m_lightStruct.orientation);
}

void AreaLightNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
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

        VulkanStructs::VDrawCallData data;
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

        // visualisation of the light
        data.effect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::DebugLine);
        if(m_visualisationMesh)
        {
            data.vertexData = &m_visualisationMesh->GetMeshData()->vertexData;
            data.indexData  = &m_visualisationMesh->GetMeshData()->indexData;
            data.indexCount = m_visualisationMesh->GetMeshIndexCount();
        }
        renderingContext->AddDrawCall(data);

        if(m_sceneNodeMetaData.IsSelected)
        {
            data.effect = effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::Outline);
            renderingContext->AddDrawCall(data);
        }
    }
}

void AreaLightNode::Update(SceneUpdateFlags& sceneUpdateFlags)
{
    LightNode<LightStructs::AreaLight>::Update(sceneUpdateFlags);
    int i                          = 0;
    m_lightStruct.position         = m_transformation->GetPosition();
    m_lightStruct.scale.x          = m_transformation->GetScale().x;
    m_lightStruct.scale.y          = m_transformation->GetScale().y;
    m_lightStruct.orientation      = m_transformation->GetRotationsQuat();
    m_lightStruct.isAreaLightInUse = m_sceneNodeMetaData.IsVisible;
    for(auto& edge : m_lightStruct.GetAreaLightEdges())
    {
        auto newEdge           = m_transformation->GetModelMatrix() * edge;
        m_lightStruct.edges[i] = newEdge;
        i++;
    }
}

void AreaLightNode::ProcessNodeRemove()
{
    m_lightStruct.Reset();
}
}  // namespace ApplicationCore