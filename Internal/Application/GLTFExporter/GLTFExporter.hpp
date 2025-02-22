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
    struct TextureBufferView;

    class GLTFExporter {
    public:
        GLTFExporter() = default;
    
        void ExportScene(std::filesystem::path path, Scene& scene,AssetsManager& assetsManager);
        
    private:
        void ParseBuffers(fastgltf::Asset& asset, AssetsManager& assetsManager);
        void ParseTexture(fastgltf::Asset& asset, AssetsManager& assetsManager);
        void ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager& assetsManager, fastgltf::Asset& asset);
        void ParseMaterial(fastgltf::Asset& asset, AssetsManager& assetsManager);
        void ParseMesh(fastgltf::Asset& asset, std::shared_ptr<StaticMesh> mesh);
        void OrganiseScene(fastgltf::Asset& asset);
        void CreateScene(fastgltf::Asset& asset,Scene& scene);
        void Clear();


    private:
        int m_nodeCounter = 0;
        std::filesystem::path m_textureDirectory;

        std::vector<TextureBufferView> m_fetchedTextureViews;
        std::unordered_map<std::string, int> m_textureToIndex;
        std::unordered_map<std::shared_ptr<Material>, int> m_materialToIndex;
        std::unordered_map<std::shared_ptr<StaticMesh>, int> m_meshToIndex;
        std::unordered_map<std::shared_ptr<SceneNode>, int> m_nodes;
        std::unordered_map<std::shared_ptr<SceneNode>, int> m_childNodes;
    };
}

#endif // GLTFEXPORTER_HPP