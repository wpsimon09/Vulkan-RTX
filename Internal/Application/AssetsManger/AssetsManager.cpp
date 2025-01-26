//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>

#include "Application/Rendering/Material/Material.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"


namespace ApplicationCore
{
    AssetsManager::AssetsManager(const VulkanCore::VDevice& device, VulkanCore::MeshDatatManager& meshDataManager):
        m_device(device), m_meshDataManager(meshDataManager), m_materials()
    {
        m_dummyTexture = std::make_shared<VulkanCore::VImage>(device);

        MaterialPaths paths{};
        m_dummyMaterial = std::make_shared<ApplicationCore::Material>(paths, *this);
    }

    void AssetsManager::DeleteAll()
    {

        m_allMeshData.clear();
        for (auto& texture : m_textures)
        {
            texture.second->Destroy();
        }
        for (auto& mesh : m_meshes)
        {
            mesh.second->Destroy();
        }
        m_dummyTexture->Destroy();
    }

    std::shared_ptr<VulkanStructs::MeshData> AssetsManager::MeshDataForGeometryType(MESH_GEOMETRY_TYPE geometryType)
    {
        auto result = m_preloadedMeshData.find(geometryType);
        // if they are loaded return the loaded result
        if (result != m_preloadedMeshData.end())
        {
            return result->second;
        }
        // load them otherwise

        switch (geometryType)
        {
        case MESH_GEOMETRY_PLANE:
            {
                auto data = m_meshDataManager.AddMeshData(MeshData::planeVertices, MeshData::planeIndices);
                m_preloadedMeshData[geometryType] = std::make_shared<VulkanStructs::MeshData>(data);
                break;
            }
        case MESH_GEOMETRY_SPHERE:
            {
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                MeshData::GenerateSphere(vertices, indices);

                auto data = m_meshDataManager.AddMeshData(vertices, indices);
                m_preloadedMeshData[geometryType] = std::make_shared<VulkanStructs::MeshData>(data);

                break;
            }
        case MESH_GEOMETRY_CUBE:
            {
                auto data = m_meshDataManager.AddMeshData(MeshData::cubeVertices, MeshData::cubeIndices);
                m_preloadedMeshData[geometryType] = std::make_shared<VulkanStructs::MeshData>(data);

                break;
            }
        case MESH_GEOMETRY_TRIANGLE:
            {
                auto data = m_meshDataManager.AddMeshData(MeshData::triangleVertices, MeshData::triangleIndices);
                m_preloadedMeshData[geometryType] = std::make_shared<VulkanStructs::MeshData>(data);
                break;
            }
        case MESH_GEOMETRY_CROSS:
            {
                auto data = m_meshDataManager.AddMeshData(MeshData::crossVertices, MeshData::crossIndices);
                m_preloadedMeshData[geometryType] = std::make_shared<VulkanStructs::MeshData>(data);
                break;
            }
        case MESH_GEOMETRY_POST_PROCESS:
            {
                auto data = m_meshDataManager.AddMeshData(MeshData::fullscreenQuadVertices,MeshData::fullscreenQuadIndices);
                m_preloadedMeshData[geometryType] = std::make_shared<VulkanStructs::MeshData>(data);
                break;
            }
        default: ;
            throw std::runtime_error("This geometry type is not supported !");
        }

        m_meshDataManager.UpdateGPU(nullptr);

        return m_preloadedMeshData[geometryType];
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(path))
        {
            if (!m_texturesToLoad.contains(path))
            {
                StartLoadingTexture(texture, path);
                m_textures[path] = std::make_shared<VulkanCore::VImage>(m_device);
            }
        }
        texture = m_textures[path];
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
                                   TextureBufferInfo& data)
    {
        // texture ID is a randomly generated string that is used to look up textures of which only data are available and no paths
        // this will most likely be used only within the editor that I plan to build in future
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(textureID))
        {
            if (!m_texturesToLoad.contains(textureID))
            {
                StartLoadingTexture(texture, textureID, data);
                m_textures[textureID] = std::make_shared<VulkanCore::VImage>(m_device);
            }
        }
        texture = m_textures[textureID];
    }

    std::shared_ptr<ApplicationCore::Material> AssetsManager::GetMaterial(MaterialPaths& path)
    {
        if (!m_materials.contains(path))
        {
            m_materials[path] = std::make_shared<Material>(path, *this);
        }
        else if (path.isTextureLess())
        {
            m_materials[path] = std::make_shared<Material>(path, *this);
        }
        return m_materials[path];
    }

    void AssetsManager::AddMesh(std::string meshName, std::shared_ptr<StaticMesh> mesh)
    {
        if (!m_meshes.contains(meshName))
        {
            m_meshes[meshName] = mesh;
        }
    }

    bool AssetsManager::Sync()
    {
        if (!m_texturesToLoad.empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            //for each texture that is being processed by separate thread
            for (auto it = m_texturesToLoad.begin(); it != m_texturesToLoad.end();)
            {
                if (it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    Utils::Logger::LogSuccess("Texture image loaded, swapping default texture for the loaded texture");
                    GlobalState::DisableLogging();
                    m_textures[it->first]->FillWithImageData(it->second.get(), true, true);
                    it = m_texturesToLoad.erase(it);
                    GlobalState::EnableLogging();
                }
                else
                {
                    ++it;
                }
            }
            return true;
        }
        Utils::Logger::LogInfoVerboseRendering("Nothing to sync...");
        return false;
    }


    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texturePtr, const std::string& path)
    {
        auto texture = std::async([this, path]()
        {
            return VulkanUtils::LoadImage(path);
        });
        m_texturesToLoad[path] = std::move(texture);
    }

    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
                                            TextureBufferInfo& data)
    {
        auto txt = std::async([this, textureID, data]()
        {
            return VulkanUtils::LoadImage(data, textureID);
        });
        m_texturesToLoad[textureID] = std::move(txt);
    }
}

// ApplicationCore
