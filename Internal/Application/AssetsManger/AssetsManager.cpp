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
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

namespace ApplicationCore
{
    AssetsManager::AssetsManager(const VulkanCore::VDevice &device):
        m_device(device) {
        m_defaultTexture = std::make_shared<VulkanCore::VImage>(m_device, "Resources/DefaultTexture.jpg");
    }

    void AssetsManager::DeleteAll() {
        for (auto &meshData : m_meshData) {
            meshData.second->Destroy();
        }
        for (auto &texture : m_textures) {
            texture.second->Destroy();
        }
        m_defaultTexture->Destroy();
    }

    VertexArray &AssetsManager::GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType) {
        auto result = m_meshData.find(geometryType);
        if (result != m_meshData.end()) {
            return *result->second;
        }
        else {
            std::unique_ptr<VertexArray> vao;
            switch (geometryType) {
            case MESH_GEOMETRY_PLANE: {
                vao = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::planeVertices,
                                                    MeshData::planeIndices);
                break;
            }
            case MESH_GEOMETRY_SPHERE: {
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                MeshData::GenerateSphere(vertices, indices);
                vao = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_STRIP, std::move(vertices),
                                                    std::move(indices));
                break;
            }
            case MESH_GEOMETRY_CUBE:
                vao = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::cubeVertices,
                                                    MeshData::cubeIndices);
                break;
            case MESH_GEOMETRY_TRIANGLE: {
                vao = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::triangleVertices,
                                                    MeshData::triangleIndices);
                break;
            }
            case MESH_GEOMETRY_CROSS: {
                vao = std::make_unique<VertexArray>(m_device, TOPOLOGY_TRIANGLE_LIST, MeshData::crossVertices,
                                                    MeshData::crossIndices);
                break;
            }
            default: ;
                throw std::runtime_error("This geometry type is not supported !");
            }
            auto inserted = m_meshData.insert(std::make_pair(geometryType, std::move(vao)));
            return *inserted.first->second;
        }
    }

    std::shared_ptr<VulkanCore::VImage> AssetsManager::GetTexture(const std::string &path) {
            if (m_textures.contains(path)) {
                return m_textures[path];
            }
            StartLoadingTexture(path);
            m_textures[path] = m_defaultTexture;

            return m_textures[path]; ;
    }


    void AssetsManager::StartLoadingTexture(const std::string &path) {
        auto texture = std::async([this, path]() {
           return std::make_shared<VulkanCore::VImage>(m_device, path);
        });
        m_textures[path] = texture.get();
    }
}



// ApplicationCore
