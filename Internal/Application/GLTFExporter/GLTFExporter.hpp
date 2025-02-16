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
struct GLTFResource
{
    const T resource;
    int index;

    GLTFResource(const T& resource, int index) : resource(resource), index(index){};
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
        void ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager& assetsManager);

        fastgltf::Node ParseNode(std::shared_ptr<SceneNode> sceneNode);

    private:
        int m_sceneNodeIndexCounter = 0;
        int m_meshIndexCounter = 0;
        int m_materialIndexCounter = 0;
        int m_imageIndexCounter = 0;

        std::vector<GLTFResource<fastgltf::Node>> m_sceneNodes;
        std::vector<GLTFResource<fastgltf::Mesh>> m_meshes;
        std::vector<GLTFResource<fastgltf::Material>> m_materials;
    };
}

#endif // GLTFEXPORTER_HPP