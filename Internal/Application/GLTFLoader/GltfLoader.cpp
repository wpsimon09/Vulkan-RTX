//
// Created by wpsimon09 on 10/12/24.
//

#include "GltfLoader.hpp"
#include <fastgltf/core.hpp>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "fastgltf/tools.hpp"

namespace ApplicationCore {

    GLTFLoader::GLTFLoader(const ApplicationCore::AssetsManager& assetsManager):m_device(assetsManager.m_device),  m_assetsManager(assetsManager)
    {
        Utils::Logger::LogSuccess("Crated GLTFLoader !");
    }

    std::shared_ptr<SceneNode> GLTFLoader::LoadGLTFScene(std::filesystem::path gltfPath)
    {

        Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

        m_rootNode = std::make_shared<SceneNode>();

        fastgltf::Parser parser {};

        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

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
            std::vector<std::shared_ptr<Mesh>> meshes;
            std::vector<std::shared_ptr<VertexArray>> vertexArrays;
            std::vector<std::shared_ptr<SceneNode>> nodes;
            std::vector<std::shared_ptr<Material>> materials;

            std::vector<uint32_t> indices;
            std::vector<Vertex> vertices;

            gltf = std::move(asset.get());
            Utils::Logger::LogSuccessClient("GLTF File parsed successfully !");

            for (fastgltf::Mesh& m: gltf.meshes)
            {
                for (auto& p : m.primitives)
                {
                    indices.clear();
                    vertices.clear();

                    //=========================================
                    // INDICES LOADING
                    //=========================================
                    fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexAccessor.count());
                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,[&](std::uint32_t index)
                    {
                       indices.push_back(index);
                    });
                }
            }
        }

        return m_rootNode;
    }
} // ApplicationCore