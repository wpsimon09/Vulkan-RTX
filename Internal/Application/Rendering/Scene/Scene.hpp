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
    void Render(VulkanStructs::RenderContext& ctx,SceneNode& sceneNode = *m_root);

private:
    void BuildDefaultScene();

private:
    std::unique_ptr<class Camera> m_camera;
    inline static std::unique_ptr<class SceneNode> m_root;
    AssetsManager& m_assetsManager;
};

} // ApplicationCore

#endif //SCENE_HPP
