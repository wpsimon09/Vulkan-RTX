//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include <memory>
#include <vector>

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/LightStructs.hpp"

namespace VulkanUtils
{
    struct RenderContext;
}

namespace ApplicationCore
{
    class DirectionLightNode;
}

namespace ApplicationCore
{
    class StaticMesh;
    class Camera;
    class SceneNode;
    class AssetsManager;
}


namespace ApplicationCore {

class Scene
{
public:
    Scene(AssetsManager& assetsManager, Camera& camera);

    void Init();
    void Update();
    void Render(VulkanUtils::RenderContext* ctx,std::shared_ptr<SceneNode> sceneNode);
    void Reset();
    void RemoveNode(SceneNode* parent, std::shared_ptr<SceneNode> nodeToRemove) const ;
    void AddNode(std::shared_ptr<SceneNode> sceneNode) const;
    AssetsManager& GetAssetsManager() const {return m_assetsManager;};

public:
    std::shared_ptr<SceneNode> GetRootNode() const {return m_root;}
    std::shared_ptr<SceneNode> GetSelectedSceneNode() const {return m_selectedSceneNode;}
    const SceneStatistics& GetSceneStatistics() const {return m_sceneStatistics;}

    void AddCubeToScene() const;
    void AddSphereToScene() const;
    void AddPlaneToScene() const;
    void AddSkyBox() const;

    void AddDirectionalLight(LightStructs::DirectionalLight* directionalLightInfo = nullptr) ;
    void AddPointLight(LightStructs::PointLight* pointLightInfo = nullptr) ;
    void AddAreaLight(LightStructs::AreaLight* areaLightInfo = nullptr) ;

    Camera& GetCamera(){return m_camera;}

    void PreformRayCast(glm::vec2 mousePosition);

    void SetSelectedSceneNode(std::shared_ptr<SceneNode> sceneNode) { m_selectedSceneNode = sceneNode; } ;

    LightStructs::SceneLightInfo& GetSceneLightInfo() {return m_sceneLightInfo;}
private:
    void BuildDefaultScene();
    SceneStatistics m_sceneStatistics;

private:
    Camera& m_camera;
    std::shared_ptr<class SceneNode> m_root;
    AssetsManager& m_assetsManager;
    glm::vec3 m_mousePositionWorldSpace  = {0.0f, 0.0f, 0.0F};
    std::shared_ptr<SceneNode> m_selectedSceneNode;

    LightStructs::SceneLightInfo m_sceneLightInfo;
};

} // ApplicationCore
#endif //SCENE_HPP
