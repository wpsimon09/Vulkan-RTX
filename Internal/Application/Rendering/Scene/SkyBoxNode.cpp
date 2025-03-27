//
// Created by wpsimon09 on 27/03/25.
//

#include "SkyBoxNode.hpp"

#include <utility>

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Rendering/Material/BaseMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {

    SkyBoxNode::SkyBoxNode(std::shared_ptr<StaticMesh> mesh):SceneNode(mesh)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::SkyBoxNode;
    }

    void SkyBoxNode::Render(ApplicationCore::EffectsLibrary& effectsLibrary,
                            VulkanUtils::RenderContext* renderingContext) const
    {
            if (!m_sceneNodeMetaData.IsVisible) return;
            VulkanStructs::DrawCallData data;
            data.firstIndex = 1;

            data.indexCount = m_mesh->GetMeshIndexCount();
           // data.indexCount_BB = m_mesh->GetMeshData()->indexData_BB.size / sizeof(uint32_t);

            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.vertexData = &m_mesh->GetMeshData()->vertexData;
            data.indexData = &m_mesh->GetMeshData()->indexData;

            data.effect = effectsLibrary.GetEffect(EEffectType::SkyBox);

            data.position = m_transformation->GetPosition();
            data.bounds = &m_mesh->GetMeshData()->bounds;
            data.material = m_mesh->GetMaterial().get();

            renderingContext->AddDrawCall(data);

    }
} // ApplicationCore