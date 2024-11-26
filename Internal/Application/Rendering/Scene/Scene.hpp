//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include <memory>
#include <vector>


namespace ApplicationCore {

class Scene {
public:
    Scene();

    void Init();
    void Update();


private:
    std::unique_ptr<class Camera> _m_camera;
    std::unique_ptr<class SceneNode> m_root;
    std::vector<std::shared_ptr<class Mesh>> m_meshes;
};

} // ApplicationCore

#endif //SCENE_HPP
