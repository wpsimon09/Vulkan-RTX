//
// Created by wpsimon09 on 04/10/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <memory>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Structs/ApplicationStructs.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"

//TODO: this class is going to boot up the client site of the renderer like geometry creation, assets managers and all of that stuff, instance of this will be creatd in VulkaRT


namespace ApplicationCore {
class ApplicationState;
}
namespace ApplicationCore {
class GLTFLoader;
}

namespace VulkanUtils {
struct RenderContext;
}

namespace VulkanStructs {
struct VDrawCallData;
}

struct CameraUpdateInfo;

namespace ApplicationCore {
class Scene;
class Camera;
}  // namespace ApplicationCore

namespace ApplicationCore {
class AssetsManager;
class StaticMesh;
class GLTFExporter;
}  // namespace ApplicationCore

class Client
{
  public:
    Client();
    void                                                                   Init();
    const std::vector<std::reference_wrapper<ApplicationCore::StaticMesh>> GetMeshes() const;
    void                                                                   Render(VulkanUtils::RenderContext* ctx);
    const void MountAssetsManger(std::unique_ptr<ApplicationCore::AssetsManager> assetsManager);
    const void Destroy();

    ApplicationCore::AssetsManager& GetAssetsManager() const { return *m_assetsManager; }

    GlobalRenderingInfo&          GetGlobalDataUpdateInformation() { return m_globalRenderingData; }
    ApplicationCore::Scene& GetScene() const { return *m_scene; };
    ;
    ApplicationCore::GLTFLoader&   GetGLTFLoader() const { return *m_gltfLoader; };
    ApplicationCore::GLTFExporter& GetGLTFExporter() const { return *m_gltfExporter; };

    PostProcessingParameters&      GetPostProcessingParameters() {return m_postProcessingParameters;}

    void Update();
    void UpdateCamera(CameraUpdateInfo& cameraUpdateInfo);
    void UpdateClient(ClientUpdateInfo& lightUpdateInfo);
    ~Client() = default;
    ApplicationCore::Camera& GetCamera() { return *m_camera; };

  public:
    bool GetIsRTXOn() const { return m_isRTXOn; };

  private:
    std::unique_ptr<ApplicationCore::AssetsManager>                 m_assetsManager;
    std::vector<std::unique_ptr<class ApplicationCore::StaticMesh>> m_meshes;
    std::unique_ptr<ApplicationCore::Camera>                        m_camera;
    std::unique_ptr<ApplicationCore::Scene>                         m_scene;
    std::unique_ptr<ApplicationCore::GLTFLoader>                    m_gltfLoader;
    std::unique_ptr<ApplicationCore::GLTFExporter>                  m_gltfExporter;
    std::unique_ptr<ApplicationCore::ApplicationState>              m_applicationState;

    GlobalRenderingInfo m_globalRenderingData;
    PostProcessingParameters m_postProcessingParameters;
    bool          m_isRTXOn = false;
};


#endif  //CLIENT_HPP
