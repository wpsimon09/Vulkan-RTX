    //
// Created by wpsimon09 on 10/12/24.
//

#include "GltfLoader.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Material/MaterialStructs.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "fastgltf/tools.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VFactories.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"
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
        m_assetsManager.m_device.GetDevice().waitIdle();
        // temp data
        std::shared_ptr<SceneNode> m_rootNode;
        std::vector<std::shared_ptr<SceneNode>> m_topNodes;
        std::vector<std::shared_ptr<SceneNode>> m_nodes;

        std::vector<std::shared_ptr<StaticMesh>> m_meshes;
        std::vector<std::shared_ptr<VulkanCore::VImage>> m_textures;
        std::vector<std::shared_ptr<Material>> materials;

        Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

        m_rootNode = std::make_unique<SceneNode>();


        fastgltf::Parser parser{};


        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::GenerateMeshIndices |
            fastgltf::Options::LoadExternalBuffers | fastgltf::Options::DecomposeNodeMatrices ;

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
                material->GetMaterialDescription().values.diffuse.a = m.pbrData.baseColorFactor.w();

                material->GetMaterialDescription().values.metalness = m.pbrData.metallicFactor;
                material->GetMaterialDescription().values.roughness = m.pbrData.roughnessFactor;



                if (m.pbrData.metallicRoughnessTexture.has_value())
                {
                    auto &textureIndex = m.pbrData.metallicRoughnessTexture.value().textureIndex;
                    if (textureIndex <= m_textures.size())
                    {
                        material->GetTexture(ETextureType::arm) = m_textures[textureIndex];
                        material->GetMaterialDescription().features.hasArmTexture = true;
                    }else
                    {
                        material->GetMaterialDescription().features.hasArmTexture = false;
                    }
                }
                if (m.pbrData.baseColorTexture.has_value())
                {
                    auto& textureIndex = m.pbrData.baseColorTexture.value().textureIndex;
                    if (textureIndex <= m_textures.size())
                    {
                        material->GetTexture(ETextureType::Diffues) = m_textures[textureIndex];
                        material->GetMaterialDescription().features.hasDiffuseTexture = true;
                    }else
                    {
                        material->GetMaterialDescription().features.hasDiffuseTexture = false;
                    }
                }
                if (m.normalTexture.has_value())
                {
                    auto& textureIndex = m.normalTexture.value().textureIndex;
                    if (textureIndex <= m_textures.size())
                    {
                        material->GetTexture(normal) = m_textures[m.normalTexture.value().textureIndex];
                        material->GetMaterialDescription().features.hasNormalTexture = true;
                    }else
                    {
                        material->GetMaterialDescription().features.hasNormalTexture = false;
                    }
                }
                material->SetMaterialname(std::string(m.name));
                
                material->SetTransparent(m.alphaMode == fastgltf::AlphaMode::Blend);
                materials.emplace_back(material);
                m_assetsManager.m_materials.emplace_back(material);
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
                indices.shrink_to_fit();

                vertices.clear();
                vertices.shrink_to_fit();
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
                    fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
                    assert(indexAccessor.bufferViewIndex.has_value() && "GLTF does not have a buffer for index data ! ");
                    indices.resize(indexAccessor.count);
                    fastgltf::copyFromAccessor<uint32_t>(gltf, indexAccessor, indices.data());
                    Utils::Logger::LogInfoVerboseOnly("Loaded indieces");

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


                auto meshData = m_assetsManager.GetBufferAllocator().AddMeshData(vertices, indices);

                // create shared ptr to mesh
                auto createdMehs = std::make_shared<StaticMesh>(meshData, mat);
                createdMehs->GeteMeshInfo().numberOfTriangles = m.primitives.size();
                createdMehs->SetName(std::string(m.name) + "##" + VulkanUtils::random_string(15));

                // store the shared ptr to mesh
                m_assetsManager.AddMesh(std::string(m.name), createdMehs);
                m_meshes.push_back(createdMehs);


                //m_rootNode->AddChild(createdMehs);
            }

            m_assetsManager.GetBufferAllocator().UpdateGPU(VK_NULL_HANDLE);

            //=====================================
            // LOAD NODES
            //=====================================
            std::shared_ptr<ApplicationCore::SceneNode> newNode;
            fastgltf::iterateSceneNodes(gltf, 0, fastgltf::math::fmat4x4(),
                            [&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix) {
                if (node.meshIndex.has_value()) {
                    newNode = std::make_shared<ApplicationCore::SceneNode>(m_meshes[node.meshIndex.value()]);
                }else
                {
                    newNode = std::make_shared<ApplicationCore::SceneNode>();

                    const auto* transform = std::get_if<fastgltf::TRS>(&node.transform);


                    Transformations transformations(
                        glm::vec3(transform->translation.x(), transform->translation.y(), transform->translation.z()),
                        glm::vec3(1.0f),
                        MathUtils::QuaternionToEuler(transform->rotation));
                    newNode->SetLocalTransform(transformations);
                }


                newNode->SetName(std::string(std::string(node.name) + "##" +VulkanUtils::random_string(4)));

                m_nodes.push_back(newNode);
            });

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