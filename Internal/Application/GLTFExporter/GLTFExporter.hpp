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
        void ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager& assetsManager, fastgltf::Asset& asset);
        void ParseMesh(fastgltf::Asset& asset, std::shared_ptr<StaticMesh> mesh);

        int FindNode(fastgltf::Asset& asset, std::shared_ptr<SceneNode> node);
        int FindMesh(fastgltf::Asset& asset, std::shared_ptr<SceneNode> node);
        int FindFindParent(fastgltf::Asset& asset, std::shared_ptr<SceneNode> node);
        std::vector<int> FindChildren(fastgltf::Asset& asset, std::shared_ptr<SceneNode> node);
    private:
        std::vector<GLTFResource<fastgltf::Mesh>> m_meshes;
        std::vector<GLTFResource<fastgltf::Material>> m_materials;
        std::vector<GLTFResource<fastgltf::Node>> m_nodes;

    };
}

#endif // GLTFEXPORTER_HPP