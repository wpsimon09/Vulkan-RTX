//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include <memory>
#include <vector>

#include "Vulkan/Global/VulkanStructs.hpp"


namespace ApplicationCore
{
    class SceneNode;
    class AssetsManager;
}

namespace ApplicationCore {

class Scene {
public:
    Scene(AssetsManager& assetsManager);

    void Init();
    void Update();
    void Render(VulkanStructs::RenderContext* ctx,SceneNode& sceneNode = *m_root);

    void AddNode(std::shared_ptr<SceneNode> sceneNode);

    void PrintSceneGraph();
public:
    std::shared_ptr<SceneNode>& GetRootNode() const {return m_root;}
private:
    void BuildDefaultScene();
    void PrintSceneDatat(int depth, SceneNode& sceneNodes);
private:
    std::unique_ptr<class Camera> m_camera;
    inline static std::shared_ptr<class SceneNode> m_root;
    AssetsManager& m_assetsManager;
};

} // ApplicationCore

#endif //SCENE_HPP
