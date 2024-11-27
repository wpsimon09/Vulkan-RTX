//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include <memory>
#include <vector>


namespace ApplicationCore
{
    class AssetsManager;
}

namespace ApplicationCore {

class Scene {
public:
    Scene(AssetsManager& assetsManager);

    void Init();
    void Update();

private:
    void BuildDefaultScene();

private:
    std::unique_ptr<class Camera> m_camera;
    std::unique_ptr<class SceneNode> m_root;
    AssetsManager& m_assetsManager;
};

} // ApplicationCore

#endif //SCENE_HPP
