//
// Created by wpsimon09 on 26/02/25.
//

#ifndef MODELIMPORTOPTIONS_HPP
#define MODELIMPORTOPTIONS_HPP
#include <filesystem>

#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore
{
    class GLTFLoader;
    class Scene;
}

namespace VEditor {
    class FileExplorer;
class ModelImportOptions: public IUserInterfaceElement {
public:
    ModelImportOptions(std::filesystem::path* path, const ApplicationCore::GLTFLoader& gltfLoader,const ApplicationCore::Scene& scene);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;
private:
    const ApplicationCore::GLTFLoader& m_gltfLoader;
    const ApplicationCore::Scene& m_scene;
    std::filesystem::path* m_path;
    ApplicationCore::ImportOptions m_options;
};

} // VEditor

#endif //MODELIMPORTOPTIONS_HPP
