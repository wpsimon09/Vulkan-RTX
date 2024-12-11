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

    std::shared_ptr<SceneNode> GLTFLoader::Load(std::filesystem::path gltfPath)
    {
        fastgltf::GltfDataBuffer data;
        data.FromPath(gltfPath);

        Utils::Logger::LogInfoClient("Loading model from path: " + gltfPath.string());

        constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

        fastgltf::Asset gltf;
        fastgltf::Parser parser {};

        auto load = parser.loadGltf(data, gltfPath.parent_path(), gltfOptions);

        if (load)
        {
            gltf = std::move(load.get());


        }else
        {
            Utils::Logger::LogErrorClient("Failed to load GLTF file: " + gltfPath.string());
            //Utils::Logger::LogInfoClient(fastgltf::getErrorMessage(load.error()));
        }

    }
} // ApplicationCore