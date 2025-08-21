//
// Created by wpsimon09 on 09/07/25.
//
#include "SceneData.hpp"

#include "FogVolumeNode.hpp"
#include "SceneNode.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include <cstddef>

namespace ApplicationCore {

void SceneData::AddEntry(std::shared_ptr<ApplicationCore::SceneNode>& node)
{
    if(node->HasMesh())
    {
        auto& mesh = node->GetMesh();
        //===================
        // for now only PBR materials will be supported
        if(auto mat = dynamic_cast<PBRMaterial*>(mesh->GetMaterial().get()))
        {
            meshes.emplace_back(mesh);
            // retrieve texture and what type of the texture it is
            auto materialTextures = mat->EnumarateTextureMap();
            int  i                = textures.size();
            for(auto& tex : materialTextures)
            {
                // assign indexes to the material struct so that we know what array it can access
                textures.emplace_back(tex.second);
                auto& material = mat->GetMaterialDescription().features;
                switch(tex.first)
                {
                    case ETextureType::Diffues:
                        material.albedo = textures.size() - 1;
                        break;
                    case ETextureType::normal:
                        material.normalTextureIdx = textures.size() - 1;
                        break;
                    case ETextureType::arm:
                        material.armTextureIdx = textures.size() - 1;
                        break;
                    case ETextureType::Emissive:
                        material.emissiveTextureIdx = textures.size() - 1;
                        break;
                }
                i++;
            }

            pbrMaterials.emplace_back(&mat->GetMaterialDescription());
            mat->SetSceneIndex(pbrMaterials.size() - 1);
        }
    }
    nodes.emplace_back(node);
    IndexNode(node);
}
bool SceneData::CheckIndexValidity(size_t arraySize, size_t index)
{
    return index < arraySize;
}

void SceneData::RemoveEntry(const ApplicationCore::SceneNode& node)
{
    if(node.HasMesh())
    {
        try
        {
            meshes.erase(meshes.begin() + node.m_meshIdx - 1);

            auto m = pbrMaterials[node.m_materialIdx];

            if(m->features.hasAlbedoTexture && CheckIndexValidity(textures.size(), m->features.albedo))
            {
                textures.erase(textures.begin() + m->features.albedo);
            }
            if(m->features.hasArmTexture && CheckIndexValidity(textures.size(), m->features.armTextureIdx))
            {
                textures.erase(textures.begin() + m->features.armTextureIdx);
            }
            if(m->features.hasEmissiveTexture && CheckIndexValidity(textures.size(), m->features.emissiveTextureIdx))
            {
                textures.erase(textures.begin() + m->features.emissiveTextureIdx);
            }
            if(m->features.hasNormalTexture && CheckIndexValidity(textures.size(), m->features.normalTextureIdx))
            {
                textures.erase(textures.begin() + m->features.normalTextureIdx);
            }

            pbrMaterials.erase(pbrMaterials.begin() + node.m_materialIdx);
        }
        catch(std::exception& e)
        {
            Utils::Logger::LogError("Failed to remove the node from the description, exceptions:" + *e.what());
        }
    }

    nodes.erase(nodes.begin() + node.m_nodeIndex);

    // reindex the scene
    for(auto& scene_node : nodes)
    {
        IndexNode(scene_node);
    }
}

void SceneData::IndexNode(std::shared_ptr<ApplicationCore::SceneNode>& node)
{
    if(node->HasMesh())
    {
        node->m_meshIdx     = meshes.size() - 1;
        node->m_materialIdx = pbrMaterials.size() - 1;
    }
    node->m_nodeIndex = nodes.size() - 1;
}

void SceneData::Reset()
{
    nodes.clear();
    meshes.clear();
    pbrMaterials.clear();
    textures.clear();
}

}  // namespace ApplicationCore
