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

//TODO: this class is going to boot up the client site of the renderer like geometry creation, assets managers and all of that stuff, instance of this will be creatd in VulkaRT


namespace VulkanStructs
{
    struct RenderContext;
}

namespace VulkanStructs
{
    struct DrawCallData;
}

struct CameraUpdateInfo;

namespace ApplicationCore
{
    class Scene;
    class Camera;
}

namespace ApplicationCore
{
    class AssetsManager;
    class Mesh;
}

class Client {
public:
    Client() = default;
    void Init();
    const std::vector<std::reference_wrapper<ApplicationCore::Mesh>> GetMeshes() const;
    const void Render(VulkanStructs::RenderContext& ctx);
    const void MountAssetsManger(std::unique_ptr<ApplicationCore::AssetsManager> assetsManager);
    const void Destroy();

    ApplicationCore::AssetsManager& GetAssetsManager() const {return *m_assetsManager;}

    glm::vec4 GetLightPosition() const {return glm::vec4(m_sunLightPosition,1.0);};

    void Update();
    void UpdateCamera(CameraUpdateInfo& cameraUpdateInfo);
    void UpdateClient(ClientUpdateInfo& lightUpdateInfo);
    ~Client() = default;
    const ApplicationCore::Camera& GetCamera() const {return *m_camera;};
public:
    bool GetIsRTXOn() const {return m_isRTXOn;};
private:
    std::unique_ptr<ApplicationCore::AssetsManager> m_assetsManager;
    std::vector<std::unique_ptr<class ApplicationCore::Mesh>> m_meshes;
    std::unique_ptr<ApplicationCore::Camera> m_camera;
    std::unique_ptr<ApplicationCore::Scene> m_scene;
    glm::vec3 m_sunLightPosition = glm::vec3(20.0f, -30.0f, -2.0f);

    bool m_isRTXOn = false;
};



#endif //CLIENT_HPP
