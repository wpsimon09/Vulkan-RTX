//
// Created by wpsimon09 on 26/11/24.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include "SceneData.hpp"


#include <memory>
#include <vector>

#include "Vulkan/Global/VulkanStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Lightning/LightStructs.hpp"

namespace VulkanCore {
namespace RTX {
struct BLASInput;
}

}  // namespace VulkanCore

namespace ApplicationCore {
class SkyBoxMaterial;
}

namespace VulkanUtils {
struct RenderContext;
}

namespace ApplicationCore {
class DirectionLightNode;
}

namespace ApplicationCore {
class StaticMesh;
class Camera;
class SceneNode;
class AssetsManager;
}  // namespace ApplicationCore


namespace ApplicationCore {


class Scene
{

  public:
    Scene(AssetsManager& assetsManager, Camera& camera);

    void Init();
    void Update();
    void Render(VulkanUtils::RenderContext* ctx, std::shared_ptr<SceneNode> sceneNode);
    void Reset();
    void RemoveNode(SceneNode* parent, std::shared_ptr<SceneNode> nodeToRemove);
    void AddNode(std::shared_ptr<SceneNode> sceneNode);
    void EnumarateMeshes(std::vector<std::shared_ptr<SceneNode>>& outMeshes, std::shared_ptr<SceneNode> sceneNode);
    std::vector<VulkanCore::RTX::BLASInput> GetBLASInputs();
    AssetsManager&                          GetAssetsManager() const { return m_assetsManager; };

  public:
    std::shared_ptr<SceneNode> GetRootNode() const { return m_root; }
    std::shared_ptr<SceneNode> GetSelectedSceneNode() const { return m_selectedSceneNode; }
    const SceneStatistics&     GetSceneStatistics() const { return m_sceneStatistics; }

    void AddCubeToScene();
    void AddSphereToScene();
    void AddPlaneToScene();
    void AddFogVolume();

    void AddSkyBox(LightStructs::EnvLight* envLight = nullptr);
    void AddDirectionalLight(LightStructs::DirectionalLight* directionalLightInfo = nullptr);
    void AddPointLight(LightStructs::PointLight* pointLightInfo = nullptr);
    void AddAreaLight(LightStructs::AreaLight* areaLightInfo = nullptr);

    Camera& GetCamera() { return m_camera; }

    void PreformRayCast(glm::vec2 mousePosition);

    void SetSelectedSceneNode(std::shared_ptr<SceneNode> sceneNode) { m_selectedSceneNode = sceneNode; };

    SceneUpdateFlags& GetSceneUpdateFlags();

    const SceneData& GetSceneDataConst() const { return m_sceneData; }
    SceneData&       GetSceneData() { return m_sceneData; }

    LightStructs::SceneLightInfo& GetSceneLightInfo() { return m_sceneLightInfo; }


  private:
    void            BuildDefaultScene();
    SceneStatistics m_sceneStatistics;

    void RebuildSceneData();

  private:
    Camera&                          m_camera;
    std::shared_ptr<class SceneNode> m_root;
    AssetsManager&                   m_assetsManager;
    glm::vec3                        m_mousePositionWorldSpace = {0.0f, 0.0f, 0.0F};
    std::shared_ptr<SceneNode>       m_selectedSceneNode;

    std::vector<std::shared_ptr<BaseMaterial>> m_sceneMaterials;

    std::vector<std::shared_ptr<StaticMesh>> m_staticMeshes;

    std::shared_ptr<SkyBoxMaterial> m_currentSkyBox;

    LightStructs::SceneLightInfo m_sceneLightInfo;

    SceneData m_sceneData;

    SceneUpdateFlags m_sceneUpdateFlags{};
};


}  // namespace ApplicationCore
#endif  //SCENE_HPP
