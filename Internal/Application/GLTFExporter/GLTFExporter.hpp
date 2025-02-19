#ifndef GLTFEXPORTER_HPP
#define GLTFEXPORTER_HPP

#include "fastgltf/types.hpp"
#include <filesystem>
#include <vector>

namespace fastgltf
{
    struct Node;
}

template<typename T>
struct GLTFResource{
    T* resource;
    int index;
};

namespace ApplicationCore {
    class StaticMesh;
    class SceneNode;
    class Scene;
    class AssetsManager;
    class Material;

    class GLTFExporter {
    public:
        GLTFExporter() = default;
    
        void ExportScene(std::filesystem::path path, Scene& scene,AssetsManager& assetsManager);
        
    private:
        void ParseBuffers(fastgltf::Asset& asset, AssetsManager& assetsManager);
        void ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager& assetsManager, fastgltf::Asset& asset);
        void ParseMesh(fastgltf::Asset& asset, std::shared_ptr<StaticMesh> mesh);
        void OrganiseScene(fastgltf::Asset& asset);

    private:
        std::unordered_map<std::shared_ptr<StaticMesh>, int> m_meshToIndex;
        std::unordered_map<std::shared_ptr<SceneNode>, int> m_sceneNodeToIndex;
    };
}

#endif // GLTFEXPORTER_HPP