//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>

#include "Application/Rendering/Material/Material.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

namespace ApplicationCore
{
    AssetsManager::AssetsManager(const VulkanCore::VDevice &device):
        m_device(device), m_materials()
    {
        m_dummyTexture = std::make_shared<VulkanCore::VImage>(device);
    }

    void AssetsManager::DeleteAll() {
        for (auto &meshData : m_preloadedMeshData) {
            meshData.second->Destroy();
        }
        for (auto &vao : m_vertexArrays) {
            vao->Destroy();
        }
        for (auto &texture : m_textures) {
            texture.second->Destroy();
        }
        for (auto& mesh: m_meshes){
            mesh.second->Destroy();
        }
        m_dummyTexture->Destroy();
    }

    std::shared_ptr<VertexArray> AssetsManager::GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType) {
        auto result = m_preloadedMeshData.find(geometryType);
        // if they are loaded return the loaded result
        if (result != m_preloadedMeshData.end()) {
            return result->second;
        }
        // load them otherwise
        std::unique_ptr<VertexArray> vao;
        switch (geometryType) {
        case MESH_GEOMETRY_PLANE: {
            m_preloadedMeshData[geometryType] = std::make_shared<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::planeVertices,
                                                MeshData::planeIndices);

            break;
        }
        case MESH_GEOMETRY_SPHERE: {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            MeshData::GenerateSphere(vertices, indices);
            m_preloadedMeshData[geometryType] =  std::make_shared<VertexArray>(m_device, TOPOLOGY_TRIANGLE_STRIP, vertices,
                                                indices);
            break;
        }
        case MESH_GEOMETRY_CUBE:
            m_preloadedMeshData[geometryType] = std::make_shared<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::cubeVertices,
                                                   MeshData::cubeIndices);
            break;
        case MESH_GEOMETRY_TRIANGLE: {
            m_preloadedMeshData[geometryType] = std::make_shared<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::triangleVertices,
                                                MeshData::triangleIndices);
            break;
        }
        case MESH_GEOMETRY_CROSS: {
            m_preloadedMeshData[geometryType] = std::make_shared<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::crossVertices,
                                                MeshData::crossIndices);
            break;
        }
        case MESH_GEOMETRY_POST_PROCESS: {
            m_preloadedMeshData[geometryType] = std::make_shared<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::fullscreenQuadVertices,
                                                MeshData::fullscreenQuadIndices);
            break;
        }
        default: ;
            throw std::runtime_error("This geometry type is not supported !");
        }
        return m_preloadedMeshData[geometryType];
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string &path) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_textures.contains(path)) {
                if (!m_texturesToLoad.contains(path)) {
                    StartLoadingTexture(texture, path);
                    m_textures[path] = std::make_shared<VulkanCore::VImage>(m_device);
                }
            }
            texture = m_textures[path];
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID ,const fastgltf::sources::Vector& data)
    {
        // texture ID is a randomly generated string that is used to look up textures of which only data are available and no paths
        // this will most likely be used only within the editor that I plan to build in future
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(textureID)) {
            if (!m_texturesToLoad.contains(textureID)) {
                StartLoadingTexture(texture, textureID);
                m_textures[textureID] = std::make_shared<VulkanCore::VImage>(m_device);
            }
        }
        texture = m_textures[textureID];
    }

    std::shared_ptr<ApplicationCore::Material> AssetsManager::GetMaterial(MaterialPaths& path)
    {
        if (!m_materials.contains(path) )
        {
            m_materials[path] = std::make_shared<Material>(path, *this);
        }
        else if (path.isTextureLess())
        {
            m_materials[path] = std::make_shared<Material>(path, *this);
        }
        return m_materials[path];
    }

    void AssetsManager::AddMesh(std::string meshName, std::shared_ptr<Mesh> mesh)
    {
        if (!m_meshes.contains(meshName))
        {
            m_meshes[meshName] = mesh;
        }
    }

    bool AssetsManager::Sync() {
        if(!m_texturesToLoad.empty()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            //for each texture that is being processed by separate thread
            for (auto it = m_texturesToLoad.begin(); it != m_texturesToLoad.end();) {
                if (it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    Utils::Logger::LogSuccess("Texture image loaded, swapping default texture for the loaded texture");
                    m_textures[it->first]->FillWithImageData(it->second.get(), true, true);
                    it = m_texturesToLoad.erase(it);
                } else {
                    ++it;
                }
            }
            return true;
        }
        Utils::Logger::LogInfoVerboseRendering("Nothing to sync...");
        return false;
    }


    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texturePtr, const std::string &path) {
        auto texture = std::async([this, path]() {
            return VulkanUtils::LoadImage(path);
        });
        m_texturesToLoad[path] = std::move(texture);
    }

    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
        const fastgltf::sources::Vector& data)
    {
        auto txt = std::async([this, textureID]() {
            return VulkanUtils::LoadImage(textureID);
        });
        m_texturesToLoad[textureID] = std::move(txt);
    }
}

// ApplicationCore
