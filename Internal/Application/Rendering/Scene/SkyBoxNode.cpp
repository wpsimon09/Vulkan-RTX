//
// Created by wpsimon09 on 27/03/25.
//

#include "SkyBoxNode.hpp"

#include <utility>

#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/BaseMaterial.hpp"
#include "Application/Rendering/Material/SkyBoxMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {

    SkyBoxNode::SkyBoxNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh):LightNode<LightStructs::EnvLight>(mesh)
    {
        m_sceneNodeMetaData.nodeType = ENodeType::SkyBoxNode;
        auto* hdrImage = dynamic_cast<ApplicationCore::SkyBoxMaterial*>(m_mesh->GetMaterial().get());
        m_lightStruct.hdrImage = hdrImage->GetHDRTexture();

        sceneLightInfo.environmentLight = &m_lightStruct;
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
            data.material = dynamic_cast<SkyBoxMaterial*>(m_mesh->GetMaterial().get());

            renderingContext->AddDrawCall(data);

    }

    void SkyBoxNode::ProcessNodeRemove()
    {
        m_lightStruct.hdrImage = nullptr;
        m_lightStruct.inUse = false;
    }

    void SkyBoxNode::Update()
    {

    }
} // ApplicationCore
