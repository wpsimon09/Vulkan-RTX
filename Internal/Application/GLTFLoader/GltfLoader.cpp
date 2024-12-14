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
#include "Application/VertexArray/VertexArray.hpp"
#include "fastgltf/tools.hpp"

namespace ApplicationCore {

    GLTFLoader::GLTFLoader(ApplicationCore::AssetsManager& assetsManager):m_device(assetsManager.m_device),  m_assetsManager(assetsManager)
    {
        Utils::Logger::LogSuccess("Crated GLTFLoader !");
    }

    std::unique_ptr<SceneNode> GLTFLoader::LoadGLTFScene(std::filesystem::path gltfPath)
    {

        Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

        m_rootNode = std::make_unique<SceneNode>();

        MaterialPaths paths;
        std::shared_ptr<Material> mat = std::make_shared<ApplicationCore::Material>(paths, m_assetsManager);

        fastgltf::Parser parser {};


        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadExternalBuffers;

        auto gltfFile = fastgltf::MappedGltfFile::FromPath(gltfPath);
        if (!bool(gltfFile))
        {
            Utils::Logger::LogErrorClient("Failed to load GLTF file: " + gltfPath.string());
            Utils::Logger::LogErrorClient( "fastgltf says: " + std::string(fastgltf::getErrorMessage(gltfFile.error())));
            return nullptr;
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

            // temporal data that will hold everything

            std::vector<uint32_t> indices;
            std::vector<Vertex> vertices;

            gltf = std::move(asset.get());
            Utils::Logger::LogSuccessClient("GLTF File parsed successfully !");

            for (fastgltf::Mesh& m: gltf.meshes)
            {
                indices.clear();
                vertices.clear();

                for (auto& p : m.primitives)
                {

                    size_t initialIndex = vertices.size();

                    //=========================================
                    // INDICES LOADING
                    //=========================================
                    {
                        fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
                        indices.reserve(indices.size() + indexAccessor.count);
                        fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,[&](std::uint32_t index)
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
                            [&](glm::vec3 v, size_t index) {
                                Vertex newvtx{};
                                newvtx.position = v;
                                // instad of use push_back or emplace_back access directly through index
                                // initial index is allways 0
                                vertices[initialIndex + index] = newvtx;
                            });

                        }else
                        {
                            Utils::Logger::LogErrorClient("Failed to find attribute 'POSITION'");
                        }
                    }

                    //=========================================
                    // VERTEX NORMALS
                    //=========================================
                    {
                        auto normals =  p.findAttribute("NORMAL");
                        if (normals != p.attributes.end())
                        {
                            auto& normalAccesor = gltf.accessors[normals->accessorIndex];
                            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, normalAccesor,
                                            [&](glm::vec3 n, size_t index) {
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
                        auto uv =  p.findAttribute("TEXCOORD_0");
                        if (uv != p.attributes.end())
                        {
                            auto& uvAccessor = gltf.accessors[uv->accessorIndex];
                            fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf,uvAccessor,
                                            [&](glm::vec2 uv, size_t index) {
                                                vertices[initialIndex + index].uv = uv;
                                            });
                        }
                        else
                        {
                            Utils::Logger::LogErrorClient("Failed to find attribute 'NORMAL'");
                        }
                    }

                }

                // store vertex array to assets manager
                m_assetsManager.GetVertexData().push_back(std::make_shared<VertexArray>(m_assetsManager.m_device, TOPOLOGY_TRIANGLE_LIST, vertices, indices));

                // create shared ptr to mesh
                auto createdMehs = std::make_shared<Mesh>(m_assetsManager.GetVertexData().back(), mat);
                createdMehs->SetName(std::string(m.name));

                // store the shared ptr to mesh
                m_assetsManager.AddMesh(std::string(m.name), createdMehs);

                m_rootNode->AddChild(createdMehs);
            }
        }

        return std::move(m_rootNode);
    }

    void GLTFLoader::PostLoadClear()
    {
        m_meshes.clear();
    }
} // ApplicationCore