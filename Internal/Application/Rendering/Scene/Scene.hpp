//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include <memory>
#include <vector>

#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"


namespace ApplicationCore
{
    class Camera;
    class SceneNode;
    class AssetsManager;
}

namespace ApplicationCore {

class Scene {
public:
    Scene(AssetsManager& assetsManager, Camera& camera);

    void Init();
    void Update();
    void Render(VulkanStructs::RenderContext* ctx,std::shared_ptr<SceneNode> sceneNode);
    void Reset();
    void RemoveNode(SceneNode* parent, std::shared_ptr<SceneNode> nodeToRemove) const ;
    void AddNode(std::shared_ptr<SceneNode> sceneNode) const;
    void PrintSceneGraph();


public:
    std::shared_ptr<SceneNode> GetRootNode() const {return m_root;}
    const SceneStatistics& GetSceneStatistics() const {return m_sceneStatistics;}

    void AddCubeToScene() const;
    void AddSphereToScene() const;
    void AddPlaneToScene() const;
    void PreformRayCast(glm::vec2 mousePosition) const;

private:
    void BuildDefaultScene();
    void PrintSceneDatat(int depth, SceneNode& sceneNodes);
    SceneStatistics m_sceneStatistics;

private:
    Camera& m_camera;
    std::shared_ptr<class SceneNode> m_root;
    AssetsManager& m_assetsManager;
    glm::vec3 m_mousePositionWorldSpace  = {0.0f, 0.0f, 0.0F};
};

} // ApplicationCore

#endif //SCENE_HPP
