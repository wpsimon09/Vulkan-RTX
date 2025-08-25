//
// Created by wpsimon09 on 26/01/25.
//

#include "DirectionLightNode.hpp"

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/LightPass.hpp"

namespace ApplicationCore {
DirectionLightNode::DirectionLightNode(LightStructs::SceneLightInfo&   sceneLightInfo,
                                       std::shared_ptr<StaticMesh>     mesh,
                                       LightStructs::DirectionalLight* directionalLightData)
    : LightNode<LightStructs::DirectionalLight>(mesh, directionalLightData)
    , m_sceneLightInfo(sceneLightInfo)
{
    m_sceneNodeMetaData.nodeType        = ENodeType::DirectionalLightNode;
    sceneLightInfo.DirectionalLightInfo = &m_lightStruct;
    m_transformation->SetPosition(m_lightStruct.direction);
    m_transformation->SetScale(1.0f);
}


void DirectionLightNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{
    if(!renderingContext->RenderBillboards)
        return;
    if (m_mesh && m_sceneNodeMetaData.IsVisible)
    {
        //==========================
        // render editor billboard
        //==========================
        // frustrum culling
        if(m_sceneNodeMetaData.FrustumCull && GlobalVariables::RenderingOptions::EnableFrustrumCulling)
        {
            if(!VulkanUtils::IsInViewFrustum (&m_mesh->GetMeshData()->bounds, m_transformation->GetModelMatrix(),
                                             renderingContext->view, renderingContext->projection))
            {
                return;
            }
        }

        VulkanStructs::VDrawCallData data;
        data.firstIndex = 1;

        data.indexCount = m_mesh->GetMeshIndexCount();
        // data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);

        data.bounds     =  &m_mesh->GetMeshData()->bounds;
        data.vertexData =  m_mesh->GetMeshData()->vertexData;
        data.indexData  =  m_mesh->GetMeshData()->indexData;

        data.modelMatrix = m_transformation->GetModelMatrix();
        data.material    = m_mesh->GetMaterial().get();
        if(renderingContext->WireFrameRendering)
            data.effect = Renderer::EForwardRenderEffects::DebugLine;
        else
            data.effect = Renderer::EForwardRenderEffects::EditorBilboard;

        data.position = m_transformation->GetPosition();

        data.bounds   =  &m_mesh->GetMeshData()->bounds;
        data.material = m_mesh->GetMaterial().get();

        renderingContext->AddDrawCall(data);

        //=======================
        // render arrow
        //=======================
        if(m_visualisationMesh)
        {
            data.effect     = Renderer::EForwardRenderEffects::DebugLine;
            data.vertexData = m_visualisationMesh->GetMeshData()->vertexData;
            data.indexData  = m_visualisationMesh->GetMeshData()->indexData;
            data.indexCount = m_visualisationMesh->GetMeshIndexCount();
            // renderingContext->AddDrawCall(data);
        }
    }
}

void DirectionLightNode::Update(SceneUpdateContext& sceneUpdateFlags)
{
    m_lightStruct.direction =
        glm::normalize(glm::vec3(m_transformation->GetRotationMatrix() * glm::vec4(glm::vec3(-1.0f, 0.0f, 0.0f), 0.0f)));


    m_lightStruct.inUse = m_sceneNodeMetaData.IsVisible;
    SceneNode::Update(sceneUpdateFlags);
}

void DirectionLightNode::ProcessNodeRemove()
{
    SceneNode::ProcessNodeRemove();
    m_lightStruct.Reset();
    m_sceneLightInfo.DirectionalLightInfo = nullptr;
}
}  // namespace ApplicationCore