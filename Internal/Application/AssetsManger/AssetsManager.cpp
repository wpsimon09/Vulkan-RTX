//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>

#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

namespace ApplicationCore
{
    AssetsManager::AssetsManager(const VulkanCore::VDevice &device):
        m_device(device) {
    }

    void AssetsManager::DeleteAll() {
        for (auto &meshData : m_meshData) {
            meshData.second->Destroy();
        }
        for (auto &texture : m_textures) {
            texture.second->Destroy();
        }
    }

    VertexArray &AssetsManager::GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType) {
        auto result = m_meshData.find(geometryType);
        // if they are loaded return the loaded result
        if (result != m_meshData.end()) {
            return *result->second;
        }
        // load them otherwise
        std::unique_ptr<VertexArray> vao;
        switch (geometryType) {
        case MESH_GEOMETRY_PLANE: {
            m_meshData[geometryType] = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::planeVertices,
                                                MeshData::planeIndices);
            break;
        }
        case MESH_GEOMETRY_SPHERE: {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            MeshData::GenerateSphere(vertices, indices);
            m_meshData[geometryType] =  std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_STRIP, vertices,
                                                indices);
            break;
        }
        case MESH_GEOMETRY_CUBE:
            m_meshData[geometryType] = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::cubeVertices,
                                                   MeshData::cubeIndices);
            break;
        case MESH_GEOMETRY_TRIANGLE: {
            m_meshData[geometryType] = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::triangleVertices,
                                                MeshData::triangleIndices);
            break;
        }
        case MESH_GEOMETRY_CROSS: {
            m_meshData[geometryType] = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::crossVertices,
                                                MeshData::crossIndices);
            break;
        }
        default: ;
            throw std::runtime_error("This geometry type is not supported !");
        }
        return *m_meshData[geometryType];
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
}

// ApplicationCore
