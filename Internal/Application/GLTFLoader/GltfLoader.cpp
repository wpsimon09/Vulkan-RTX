//
// Created by wpsimon09 on 10/12/24.
//

#include "GltfLoader.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Material/MaterialStructs.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Application/Utils/ModelExportImportUtils/ModelManagmentUtils.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "fastgltf/tools.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "imgui_internal.h"

namespace ApplicationCore {
GLTFLoader::GLTFLoader(ApplicationCore::AssetsManager& assetsManager)
    : m_device(assetsManager.m_device)
    , m_assetsManager(assetsManager)
{
    Utils::Logger::LogSuccess("Crated GLTFLoader !");
}

void GLTFLoader::LoadGLTFScene(Scene& scene, std::filesystem::path gltfPath, const ImportOptions& importOptions) const
{
    const auto& model = m_assetsManager.GetModel(gltfPath.string());
    if(!model.empty())
        return;

    // temp data
    std::shared_ptr<SceneNode>              m_rootNode;
    std::vector<std::shared_ptr<SceneNode>> m_topNodes;
    std::vector<std::shared_ptr<SceneNode>> m_nodes;

    std::vector<std::shared_ptr<StaticMesh>>                     m_meshes;
    std::vector<std::shared_ptr<ApplicationCore::VTextureAsset>> m_textures;
    std::vector<std::shared_ptr<PBRMaterial>>                    materials;

    Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

    m_rootNode = std::make_unique<SceneNode>();


    fastgltf::Parser parser{};


    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble
                                 | fastgltf::Options::GenerateMeshIndices | fastgltf::Options::LoadExternalBuffers
                                 | fastgltf::Options::DecomposeNodeMatrices;

    auto gltfFile = fastgltf::MappedGltfFile::FromPath(gltfPath);
    if(!bool(gltfFile))
    {
        Utils::Logger::LogErrorClient("Failed to load GLTF file: " + gltfPath.string());
        Utils::Logger::LogErrorClient("fastgltf says: " + std::string(fastgltf::getErrorMessage(gltfFile.error())));
        return;
    }

    fastgltf::Asset gltf;

    auto asset = parser.loadGltf(gltfFile.get(), gltfPath.parent_path(), gltfOptions);
    if(asset.error() != fastgltf::Error::None)
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
        if(importOptions.importMaterials)
        {
            for(auto& image : gltf.images)
            {
                LoadImage(gltf, gltfPath.parent_path(), image, m_textures, true);
            }

            //m_assetsManager.Sync();

            //==============================================================
            // MATERIAL
            //==============================================================

            for(fastgltf::Material& m : gltf.materials)
            {
                MaterialPaths                paths    = {.saveToDisk = true};
                std::shared_ptr<PBRMaterial> material = std::make_shared<ApplicationCore::PBRMaterial>(
                    m_assetsManager.GetAllRasterEffects()[EEffectType::ForwardShader], paths, m_assetsManager);
                material->SetSavable(true);
                material->GetMaterialDescription().values.albedo.x = m.pbrData.baseColorFactor.x();
                material->GetMaterialDescription().values.albedo.y = m.pbrData.baseColorFactor.y();
                material->GetMaterialDescription().values.albedo.z = m.pbrData.baseColorFactor.z();
                material->GetMaterialDescription().values.albedo.a = m.pbrData.baseColorFactor.w();

                material->GetMaterialDescription().values.emissive_strength.x = m.emissiveFactor.x();
                material->GetMaterialDescription().values.emissive_strength.y = m.emissiveFactor.y();
                material->GetMaterialDescription().values.emissive_strength.z = m.emissiveFactor.z();
                material->GetMaterialDescription().values.emissive_strength.w = m.emissiveStrength;

                material->GetMaterialDescription().values.metalness = m.pbrData.metallicFactor;
                material->GetMaterialDescription().values.roughness = m.pbrData.roughnessFactor;

                if(m.pbrData.metallicRoughnessTexture.has_value())
                {
                    auto& textureIndex = m.pbrData.metallicRoughnessTexture.value().textureIndex;
                    if(textureIndex <= m_textures.size())
                    {
                        material->GetTexture(ETextureType::arm) = m_textures[textureIndex];
                        material->GetMaterialPaths().ArmMapPath = m_textures[textureIndex]->GetAssetPath();
                        material->GetMaterialDescription().features.hasArmTexture = true;
                    }
                    else
                    {
                        material->GetMaterialDescription().features.hasArmTexture = false;
                    }
                }
                if(m.pbrData.baseColorTexture.has_value())
                {
                    auto& textureIndex = m.pbrData.baseColorTexture.value().textureIndex;
                    if(textureIndex < m_textures.size())
                    {
                        material->GetTexture(ETextureType::Diffues) = m_textures[textureIndex];
                        material->GetMaterialPaths().DiffuseMapPath = m_textures[textureIndex]->GetAssetPath();
                        material->GetMaterialDescription().features.hasAlbedoTexture = true;
                    }
                    else
                    {
                        material->GetMaterialDescription().features.hasAlbedoTexture = false;
                    }
                }
                if(m.emissiveTexture.has_value())
                {
                    auto& textureIndex = m.emissiveTexture.value().textureIndex;
                    if(textureIndex < m_textures.size())
                    {
                        material->GetTexture(ETextureType::Emissive) = m_textures[textureIndex];
                        material->GetMaterialPaths().EmmisivePath    = m_textures[textureIndex]->GetAssetPath();
                        material->GetMaterialDescription().features.hasEmissiveTexture = true;
                    }
                    else
                    {
                        material->GetMaterialDescription().features.hasEmissiveTexture = false;
                    }
                }
                if(m.normalTexture.has_value())
                {
                    auto& textureIndex = m.normalTexture.value().textureIndex;
                    if(textureIndex <= m_textures.size())
                    {
                        material->GetTexture(normal) = m_textures[m.normalTexture.value().textureIndex];
                        material->GetMaterialPaths().NormalMapPath =
                            m_textures[m.normalTexture.value().textureIndex]->GetAssetPath();
                        material->GetMaterialDescription().features.hasNormalTexture = true;
                    }
                    else
                    {
                        material->GetMaterialDescription().features.hasNormalTexture = false;
                    }
                }
                material->SetMaterialname(std::string(m.name) + "##" + VulkanUtils::random_string(4));

                if(m.alphaMode == fastgltf::AlphaMode::Blend)
                {
                    material->ChangeEffect(std::dynamic_pointer_cast<VulkanUtils::VRasterEffect>(
                        m_assetsManager.GetEffects()[EEffectType::AplhaBlend]));
                }
                else if(m.alphaMode == fastgltf::AlphaMode::Mask)
                {
                    material->ChangeEffect(std::dynamic_pointer_cast<VulkanUtils::VRasterEffect>(
                        m_assetsManager.GetEffects()[EEffectType::AplhaBlend]));
                }
                material->SetTransparent(m.alphaMode == fastgltf::AlphaMode::Blend);
                materials.emplace_back(material);
                m_assetsManager.m_materials.emplace_back(material);
            }
        }
        else
        {

            MaterialPaths                paths    = {.saveToDisk = true};
            std::shared_ptr<PBRMaterial> material = std::make_shared<ApplicationCore::PBRMaterial>(
                m_assetsManager.GetAllRasterEffects()[EEffectType::ForwardShader], paths, m_assetsManager);
            materials.emplace_back(material);
            m_assetsManager.m_materials.emplace_back(material);
        }

        //==============================================================
        // MESHES LOADING
        //==============================================================
        if(importOptions.importOnlyMaterials)
        {
            return;
        }
        // temporal data that will hold everything
        std::vector<uint32_t> indices;
        std::vector<Vertex>   vertices;

        for(fastgltf::Mesh& m : gltf.meshes)
        {
            indices.clear();
            indices.shrink_to_fit();

            vertices.clear();
            vertices.shrink_to_fit();
            VulkanStructs::VBounds bounds = {};

            MaterialPaths paths;

            std::shared_ptr<PBRMaterial> mat = std::make_shared<ApplicationCore::PBRMaterial>(
                m_assetsManager.GetAllRasterEffects()[EEffectType::ForwardShader], paths, m_assetsManager);


            bool hasTangents = false;

            for(auto& p : m.primitives)
            {
                if(!importOptions.importMaterials)
                {
                    p.materialIndex = 0;  // first element in materials array
                }
                if(p.materialIndex.has_value())
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
                    if(positioIt)
                    {
                        auto& posAccessor = gltf.accessors[positioIt->accessorIndex];
                        // resize from 0 to number of vertices
                        if(!posAccessor.bufferViewIndex.has_value())
                            continue;

                        vertices.resize(vertices.size() + posAccessor.count);

                        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](glm::vec3 v, size_t index) {
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
                    if(normals != p.attributes.end())
                    {
                        auto& normalAccesor = gltf.accessors[normals->accessorIndex];
                        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, normalAccesor, [&](glm::vec3 n, size_t index) {
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
                    if(uv != p.attributes.end())
                    {
                        hasTangents      = true;
                        auto& uvAccessor = gltf.accessors[uv->accessorIndex];
                        fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, uvAccessor, [&](glm::vec2 uv, size_t index) {
                            vertices[initialIndex + index].uv = uv;
                        });
                    }
                    else
                    {
                        hasTangents = false;
                        Utils::Logger::LogErrorClient("Failed to find attribute 'TEXTURE COORD'");
                    }
                }
                //===========================================
                // VERTEX TANGENTS
                //===========================================
                {
                    auto tangents = p.findAttribute("TANGENT");
                    if(tangents != p.attributes.end())
                    {
                        auto& tangentAccessor = gltf.accessors[tangents->accessorIndex];

                        fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, tangentAccessor, [&](glm::vec4 t, size_t index) {
                            vertices[initialIndex + index].tangent = t;
                        });
                    }
                    else
                    {
                        Utils::Logger::LogErrorClient("Failed to find attribute 'TANGENT'");
                    }
                }

                // calculate the tangents and store them directily to the vertices provided
                if(!hasTangents)
                {
                    Utils::Logger::LogInfo("No tangent vectors found, calculating them now !");
                    auto result = ApplicationCore::GeneratTangents(vertices, indices);
                    assert(result = true && "Failed to generate tangent vectors ! ");
                }

                // store vertex array to assets manager
                auto meshData = m_assetsManager.GetBufferAllocator().AddMeshData(vertices, indices);

                // create shared ptr to mesh
                auto createdMehs                              = std::make_shared<StaticMesh>(meshData, mat);
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
            fastgltf::iterateSceneNodes(gltf, 0, fastgltf::math::fmat4x4(), [&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix) {
                if(node.meshIndex.has_value())
                {
                    newNode = std::make_shared<ApplicationCore::SceneNode>(m_meshes[node.meshIndex.value()]);
                }
                else
                {
                    newNode = std::make_shared<ApplicationCore::SceneNode>();
                }

                const auto* transform = std::get_if<fastgltf::TRS>(&node.transform);


                glm::quat rotation(transform->rotation.w(), transform->rotation.x(), transform->rotation.y(),
                                   transform->rotation.z());

                Transformations transformations(
                    glm::vec3(transform->translation.x(), transform->translation.y(), transform->translation.z()),
                    glm::vec3(transform->scale.x(), transform->scale.y(), transform->scale.z()), rotation);

                newNode->SetLocalTransform(transformations);

                newNode->SetName(std::string(std::string(node.name) + "##" + VulkanUtils::random_string(4)));

                m_nodes.push_back(newNode);
            });
        }

        // construct the hierarchy
        for(int i = 0; i < gltf.nodes.size(); i++)
        {
            std::shared_ptr<SceneNode> sceneNode = m_nodes[i];

            for(auto c : gltf.nodes[i].children)
            {
                sceneNode->AddChild(scene.GetSceneData(), m_nodes[c]);
            }
        }

        for(auto& m_node : m_nodes)
        {
            if(m_node->IsParent())
            {
                m_topNodes.push_back(m_node);
            }
        }

        for(auto& topNode : m_topNodes)
        {
            auto newScale = topNode->m_transformation->GetScale() * importOptions.uniformScale;
            topNode->m_transformation->SetScale(newScale);
            m_assetsManager.AddModel(gltfPath.string() + "/" + topNode->GetName(), topNode->GetChildrenByRef());
        }

        for(auto& sceneNode : m_topNodes)
        {
            scene.AddNode(sceneNode);
        }

        scene.Update();

        GlobalState::EnableLogging();
        Utils::Logger::LogSuccess("Model at path" + gltfPath.string() + "was loaded successfully");
    }


    void GLTFLoader::LoadImage(fastgltf::Asset & asset, std::string parentPath, fastgltf::Image & image,
                               std::vector<std::shared_ptr<ApplicationCore::VTextureAsset>> & imageStorage, bool saveToDisk) const
    {
        std::visit(fastgltf::visitor{
                       [](auto& arg) {},

                       [&](fastgltf::sources::URI& filePath) {
                           std::shared_ptr<ApplicationCore::VTextureAsset> loadedTexture;
                           const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());
                           m_assetsManager.GetTexture(loadedTexture, parentPath + "/" + path, saveToDisk);
                           imageStorage.emplace_back(loadedTexture);
                       },

                       [&](fastgltf::sources::Vector& vector) {
                           std::shared_ptr<ApplicationCore::VTextureAsset> loadedTexture;
                           const std::string                               textureID = VulkanUtils::random_string(4);
                           TextureBufferInfo                               bufferInfo{};
                           bufferInfo.data = vector.bytes.data();
                           bufferInfo.size = vector.bytes.size();

                           m_assetsManager.GetTexture(loadedTexture, textureID, bufferInfo, saveToDisk);
                           imageStorage.emplace_back(loadedTexture);
                       },

                       [&](fastgltf::sources::BufferView& view) {
                           auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                           auto& buffer     = asset.buffers[bufferView.bufferIndex];

                           std::visit(fastgltf::visitor{
                                          // We only care about VectorWithMime here, because we
                                          // specify LoadExternalBuffers, meaning all buffers
                                          // are already loaded into a vector.
                                          [](auto& arg) {},
                                          [&](fastgltf::sources::Vector& vector) {
                                              std::shared_ptr<ApplicationCore::VTextureAsset> loadedTexture;
                                              const std::string textureID = VulkanUtils::random_string(10);
                                              TextureBufferInfo textureBufferInfo{};
                                              textureBufferInfo.data = vector.bytes.data() + bufferView.byteOffset;
                                              textureBufferInfo.size = vector.bytes.size();

                                              if(image.name.empty())
                                              {
                                                  image.name = textureID;
                                              }
                                              auto name = std::string(image.name.c_str()) + ".png";

                                              m_assetsManager.GetTexture(loadedTexture, name, textureBufferInfo, saveToDisk);
                                              imageStorage.emplace_back(loadedTexture);
                                          },
                                          [&](fastgltf::sources::Array& vector) {
                                              std::shared_ptr<ApplicationCore::VTextureAsset> loadedTexture;
                                              const std::string textureID = VulkanUtils::random_string(10);
                                              TextureBufferInfo textureBufferInfo{};
                                              textureBufferInfo.data = vector.bytes.data() + bufferView.byteOffset;
                                              textureBufferInfo.size = vector.bytes.size();

                                              if(image.name.empty())
                                              {
                                                  image.name = textureID;
                                              }
                                              auto name                   = std::string(image.name.c_str()) + ".png";
                                              textureBufferInfo.textureID = name;


                                              m_assetsManager.GetTexture(loadedTexture, name, textureBufferInfo, saveToDisk);
                                              imageStorage.emplace_back(loadedTexture);
                                          }},
                                      buffer.data);
                       },
                   },
                   image.data);
    }
}  // namespace ApplicationCore