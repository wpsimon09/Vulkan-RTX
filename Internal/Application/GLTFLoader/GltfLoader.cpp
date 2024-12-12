//
// Created by wpsimon09 on 10/12/24.
//

#include "GltfLoader.hpp"
#include <fastgltf/core.hpp>

#include "Application/AssetsManger/AssetsManager.hpp"

namespace ApplicationCore {

    GLTFLoader::GLTFLoader(const ApplicationCore::AssetsManager& assetsManager):m_device(assetsManager.m_device),  m_assetsManager(assetsManager)
    {
        Utils::Logger::LogSuccess("Crated GLTFLoader !");
    }

    std::shared_ptr<SceneNode> GLTFLoader::LoadGLTFScene(std::filesystem::path gltfPath)
    {
        fastgltf::GltfDataBuffer data;
        data.FromPath(gltfPath);

        Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

        constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

        fastgltf::Asset gltf;
        fastgltf::Parser parser {};

        auto type = fastgltf::determineGltfFileType(data);

        if (type == fastgltf::GltfType::glTF)
        {
            auto load = parser.loadGltfJson(data, gltfPath.parent_path(), gltfOptions);
            if (load)
            {
                gltf = std::move(load.get());
            }
            else
            {
                Utils::Logger::LogErrorClient("Failed to load GLTF file: " + gltfPath.string());
                Utils::Logger::LogErrorClient( "fastgltf says: " + std::string(fastgltf::getErrorMessage(load.error())));
            }
        }if ()


        return nullptr;
    }
} // ApplicationCore