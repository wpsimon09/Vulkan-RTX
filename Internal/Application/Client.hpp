//
// Created by wpsimon09 on 04/10/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <memory>
#include <vector>

//TODO: this class is going to boot up the client site of the renderer like geometry creation, assets managers and all of that stuff, instance of this will be creatd in VulkaRT


struct CameraUpdateInfo;

namespace ApplicationCore
{
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
    const void MountAssetsManger(std::unique_ptr<ApplicationCore::AssetsManager> assetsManager);
    const void Destroy();
    void Update(const CameraUpdateInfo& cameraUpdateInfo);
    ~Client() = default;
private:
    std::unique_ptr<ApplicationCore::AssetsManager> m_assetsManager;
    std::vector<std::unique_ptr<class ApplicationCore::Mesh>> m_meshes;
    std::unique_ptr<ApplicationCore::Camera> m_camera;
};



#endif //CLIENT_HPP
