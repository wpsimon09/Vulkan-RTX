//
// Created by wpsimon09 on 10/12/24.
//

#include "GltfLoader.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Material/MaterialStructs.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "fastgltf/tools.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace ApplicationCore
{
    GLTFLoader::GLTFLoader(ApplicationCore::AssetsManager& assetsManager): m_device(assetsManager.m_device),
                                                                           m_assetsManager(assetsManager)
    {
        Utils::Logger::LogSuccess("Crated GLTFLoader !");
    }

    std::vector<std::shared_ptr<SceneNode>> GLTFLoader::LoadGLTFScene(std::filesystem::path gltfPath) const
    {
        // temp data
        std::shared_ptr<SceneNode> m_rootNode;
        std::vector<std::shared_ptr<SceneNode>> m_topNodes;
        std::vector<std::shared_ptr<SceneNode>> m_nodes;

        std::vector<std::shared_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<VertexArray>> vertexArrays;
        std::vector<std::shared_ptr<VulkanCore::VImage>> m_textures;
        std::vector<std::shared_ptr<Material>> materials;

        Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

        m_rootNode = std::make_unique<SceneNode>();


        fastgltf::Parser parser{};


        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::GenerateMeshIndices |
            fastgltf::Options::LoadExternalBuffers;

        auto gltfFile = fastgltf::MappedGltfFile::FromPath(gltfPath);
        if (!bool(gltfFile))
        {
            Utils::Logger::LogErrorClient("Failed to load GLTF file: " + gltfPath.string());
            Utils::Logger::LogErrorClient("fastgltf says: " + std::string(fastgltf::getErrorMessage(gltfFile.error())));
            return std::vector<std::shared_ptr<SceneNode>>();
        }

        fastgltf::Asset gltf;

        auto asset = parser.loadGltf(gltfFile.get(), gltfPath.parent_path(), gltfOptions);
        if (asset.error() != fastgltf::Error::None)
        {
            Utils::Logger::LogErrorClient("Failed to parse GLTF file " + gltfPath.string());
            Utils::Logger::LogErrorClient("fastgltf says: " + std::string(fastgltf::getErrorMessage(gltfFile.error())));
        }
        else
        {
            gltf = std::move(asset.get());
            Utils::Logger::LogSuccessClient("GLTF File parsed successfully !");

            GlobalState::DisableLogging();
            //==============================================================
            // TEXTURE LOADING
            //==============================================================
            for (auto& image : gltf.images)
            {
                LoadImage(gltf, gltfPath.parent_path(), image, m_textures);
            }

            //==============================================================
            // MATERIAL
            //==============================================================
            for (fastgltf::Material& m : gltf.materials)
            {
                MaterialPaths paths = {};
                std::shared_ptr<Material> material = std::make_shared<
                    ApplicationCore::Material>(paths, m_assetsManager);
                material->GetMaterialDescription().values.diffuse.x = m.pbrData.baseColorFactor.x();
                material->GetMaterialDescription().values.diffuse.y = m.pbrData.baseColorFactor.y();
                material->GetMaterialDescription().values.diffuse.z = m.pbrData.baseColorFactor.z();

                material->GetMaterialDescription().values.metalness = m.pbrData.metallicFactor;
                material->GetMaterialDescription().values.roughness = m.pbrData.roughnessFactor;
                //material->GetMaterialDescription().values.ao = m.pbrData.

                if (m.pbrData.metallicRoughnessTexture.has_value())
                {
                    material->GetTexture(MATERIAL_TYPE::PBR_ARM) = m_textures[m.pbrData.metallicRoughnessTexture.value()
                                                                               .textureIndex];
                    material->GetMaterialDescription().features.hasArmTexture = true;
                }
                if (m.pbrData.baseColorTexture.has_value())
                {
                    material->GetTexture(MATERIAL_TYPE::PBR_DIFFUSE_MAP) = m_textures[m.pbrData.baseColorTexture.value()
                        .textureIndex];
                    material->GetMaterialDescription().features.hasDiffuseTexture = true;
                }
                if (m.normalTexture.has_value())
                {
                    material->GetTexture(PBR_NORMAL_MAP) = m_textures[m.normalTexture.value().textureIndex];
                    material->GetMaterialDescription().features.hasNormalTexture = true;
                }

                materials.emplace_back(material);
            }

            //==============================================================
            // MESHES LOADING
            //==============================================================

            // temporal data that will hold everything
            std::vector<uint32_t> indices;
            std::vector<Vertex> vertices;

            for (fastgltf::Mesh& m : gltf.meshes)
            {
                indices.clear();
                vertices.clear();
                VulkanStructs::Bounds bounds = {};

                MaterialPaths paths;

                std::shared_ptr<Material> mat = std::make_shared<ApplicationCore::Material>(paths, m_assetsManager);


                for (auto& p : m.primitives)
                {
                    if (p.materialIndex.has_value())
                    {
                        mat = materials[p.materialIndex.value()];
                    }

                    size_t initialIndex = vertices.size();

                    //=========================================
                    // INDICES LOADING
                    //=========================================
                    {
                        fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
                        indices.reserve(indices.size() + indexAccessor.count);
                        fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor, [&](std::uint32_t index)
                        {
                            indices.push_back(index);
                        });
                    }

                    //=========================================
                    // VERTEX POSITIONS
                    //=========================================
                    {
                        auto* positioIt = p.findAttribute("POSITION");
                        if (positioIt)
                        {
                            auto& posAccessor = gltf.accessors[positioIt->accessorIndex];
                            // resize from 0 to number of vertices
                            if (!posAccessor.bufferViewIndex.has_value())
                                continue;

                            vertices.resize(vertices.size() + posAccessor.count);

                            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                                                                          [&](glm::vec3 v, size_t index)
                                                                          {
                                                                              Vertex newvtx{};
                                                                              newvtx.position = v;
                                                                              // instad of use push_back or emplace_back access directly through index
                                                                              // initial index is allways 0
                                                                              vertices[initialIndex + index] = newvtx;
                                                                          });
                        }
                        else
                        {
                            Utils::Logger::LogErrorClient("Failed to find attribute 'POSITION'");
                        }
                    }

                    //=========================================
                    // VERTEX NORMALS
                    //=========================================
                    {
                        auto normals = p.findAttribute("NORMAL");
                        if (normals != p.attributes.end())
                        {
                            auto& normalAccesor = gltf.accessors[normals->accessorIndex];
                            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, normalAccesor,
                                                                          [&](glm::vec3 n, size_t index)
                                                                          {
                                                                              vertices[initialIndex + index].normal = n;
                                                                          });
                        }
                        else
                        {
                            Utils::Logger::LogErrorClient("Failed to find attribute 'NORMAL'");
                        }
                    }

                    //=========================================
                    // TEXTURE COORDINATES
                    //=========================================
                    {
                        auto uv = p.findAttribute("TEXCOORD_0");
                        if (uv != p.attributes.end())
                        {
                            auto& uvAccessor = gltf.accessors[uv->accessorIndex];
                            fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, uvAccessor,
                                                                          [&](glm::vec2 uv, size_t index)
                                                                          {
                                                                              vertices[initialIndex + index].uv = uv;
                                                                          });
                        }
                        else
                        {
                            Utils::Logger::LogErrorClient("Failed to find attribute 'TEXTURE COORD'");
                        }
                    }
                }

                // store vertex array to assets manager
                m_assetsManager.GetVertexData().push_back(
                    std::make_shared<VertexArray>(m_assetsManager.m_device, TOPOLOGY_TRIANGLE_LIST, vertices, indices));

                // create shared ptr to mesh
                auto createdMehs = std::make_shared<Mesh>(m_assetsManager.GetVertexData().back(), mat);
                createdMehs->GeteMeshInfo().numberOfTriangles = m.primitives.size();
                createdMehs->SetName(std::string(m.name) + "##" + VulkanUtils::random_string(15));

                // store the shared ptr to mesh
                m_assetsManager.AddMesh(std::string(m.name), createdMehs);
                m_meshes.push_back(createdMehs);


                //m_rootNode->AddChild(createdMehs);
            }

            //=====================================
            // LOAD NODES
            //=====================================
            for (auto& node : gltf.nodes)
            {
                std::shared_ptr<ApplicationCore::SceneNode> newNode;
                if (node.meshIndex.has_value())
                {
                    newNode = std::make_shared<ApplicationCore::SceneNode>(m_meshes[node.meshIndex.value()]);
                }
                else
                {
                    newNode = std::make_shared<ApplicationCore::SceneNode>();
                }
                newNode->SetName(std::string(std::string(node.name) + "##" +VulkanUtils::random_string(4)));
                if (auto newTransform = std::get_if<fastgltf::math::fmat4x4>(&node.transform))
                {
                    //newNode->m_transformation->SetModelMatrix(VulkanUtils::FastGLTFToGLMMat4(*newTransform));
                }
                m_nodes.push_back(newNode);
            }
        }

        // construct the hierarchy
        for (int i = 0; i < gltf.nodes.size(); i++)
        {
            std::shared_ptr<SceneNode> sceneNode = m_nodes[i];

            for (auto c : gltf.nodes[i].children)
            {
                sceneNode->AddChild(m_nodes[c]);
            }
        }

        for (auto& m_node : m_nodes)
        {
            if (m_node->IsParent())
            {
                m_topNodes.push_back(m_node);
            }
        }

        for (const auto& topNode : m_topNodes)
        {
            //m_rootNode->AddChild(topNode);
        }

        GlobalState::EnableLogging();
        Utils::Logger::LogSuccess("Model at path" + gltfPath.string() + "was loaded successfully");
        return std::move(m_topNodes);
    }

    void GLTFLoader::LoadImage(fastgltf::Asset& asset, std::string parentPath, fastgltf::Image& image,
                               std::vector<std::shared_ptr<VulkanCore::VImage>>& imageStorage) const
    {
        std::visit(
            fastgltf::visitor{
                [](auto& arg)
                {
                },

                [&](fastgltf::sources::URI& filePath)
                {
                    std::shared_ptr<VulkanCore::VImage> loadedTexture;
                    const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());
                    m_assetsManager.GetTexture(loadedTexture, parentPath + "/" + path);
                    imageStorage.emplace_back(loadedTexture);
                },

                [&](fastgltf::sources::Vector& vector)
                {
                    std::shared_ptr<VulkanCore::VImage> loadedTexture;
                    const std::string textureID = VulkanUtils::random_string(4);
                    TextureBufferInfo bufferInfo{};
                    bufferInfo.data = vector.bytes.data();
                    bufferInfo.size = vector.bytes.size();

                    m_assetsManager.GetTexture(loadedTexture, textureID, bufferInfo);
                    imageStorage.emplace_back(loadedTexture);
                },

                [&](fastgltf::sources::BufferView& view)
                {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer = asset.buffers[bufferView.bufferIndex];

                    std::visit(fastgltf::visitor{
                                   // We only care about VectorWithMime here, because we
                                   // specify LoadExternalBuffers, meaning all buffers
                                   // are already loaded into a vector.
                                   [](auto& arg)
                                   {
                                   },
                                   [&](fastgltf::sources::Vector& vector)
                                   {
                                       std::shared_ptr<VulkanCore::VImage> loadedTexture;
                                       const std::string textureID = VulkanUtils::random_string(10);
                                       TextureBufferInfo textureBufferInfo{};
                                       textureBufferInfo.data = vector.bytes.data() + bufferView.byteOffset;
                                       textureBufferInfo.size = vector.bytes.size();

                                       m_assetsManager.GetTexture(loadedTexture, textureID, textureBufferInfo);
                                       imageStorage.emplace_back(loadedTexture);
                                   },
                                    [&](fastgltf::sources::Array& vector)
                                   {
                                       std::shared_ptr<VulkanCore::VImage> loadedTexture;
                                       const std::string textureID = VulkanUtils::random_string(10);
                                       TextureBufferInfo textureBufferInfo{};
                                       textureBufferInfo.data = vector.bytes.data() + bufferView.byteOffset;
                                       textureBufferInfo.size = vector.bytes.size();

                                       m_assetsManager.GetTexture(loadedTexture, textureID, textureBufferInfo);
                                       imageStorage.emplace_back(loadedTexture);
                                   }
                               },
                               buffer.data);
                },
            }, image.data);
    }
} // ApplicationCore
