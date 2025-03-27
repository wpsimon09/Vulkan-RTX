//
// Created by wpsimon09 on 27/03/25.
//

#include "SkyBoxNode.hpp"

#include <utility>

#include "Application/Rendering/Mesh/StaticMesh.hpp"

namespace ApplicationCore {

    SkyBoxNode::SkyBoxNode(std::shared_ptr<StaticMesh> mesh):SceneNode(mesh)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::SkyBoxNode;
    }

    void SkyBoxNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary,
                            VulkanUtils::RenderContext* renderingContext) const
    {
                    VulkanStructs::DrawCallData data;
            data.firstIndex = 1;

            data.indexCount = m_mesh->GetMeshIndexCount();
           // data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);

            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.vertexData = &m_mesh->GetMeshData()->vertexData;
            data.indexData = &m_mesh->GetMeshData()->indexData;

            data.modelMatrix = m_transformation->GetModelMatrix();
            if (renderingContext->WireFrameRendering)
                data.effect = effectsLibrary.GetEffect(EEffectType::DebugLine);
            else if (m_mesh->m_currentMaterial->IsTransparent())
            {
                data.effect = effectsLibrary.GetEffect(EEffectType::AplhaBlend);
            }
            else
            {
                //data.effect = effectsLibrary.GetEffect(EEffectType::ForwardShader);
                data.effect = m_mesh->GetMaterial()->GetEffect();

            }

            data.position = m_transformation->GetPosition();
            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.material = m_mesh->m_currentMaterial.get();

            renderingContext->AddDrawCall(data);

            if (m_sceneNodeMetaData.IsSelected)
            {
                data.effect = effectsLibrary.GetEffect(EEffectType::Outline);
                renderingContext->AddDrawCall(data);
            }

            //=====================================================
            // BOUNDING VOLUME STUFF
            //=====================================================
            if (renderingContext->RenderAABB)
            {
                data.vertexData = &m_mesh->GetMeshData()->vertexData_BB;
                data.indexData = &m_mesh->GetMeshData()->indexData_BB;
                data.indexCount = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);;
                data.effect = effectsLibrary.GetEffect(EEffectType::DebugLine);
                renderingContext->AddDrawCall(data);

            }

            if (m_sceneNodeMetaData.CastsShadows)
            {
                // change effect
            }

    }
} // ApplicationCore