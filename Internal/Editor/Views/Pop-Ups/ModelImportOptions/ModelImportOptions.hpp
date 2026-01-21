//
// Created by wpsimon09 on 26/02/25.
//

#ifndef MODELIMPORTOPTIONS_HPP
#define MODELIMPORTOPTIONS_HPP
#include <filesystem>

#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore {
class GLTFLoader;
class Scene;
}  // namespace ApplicationCore

namespace VEditor {
class FileExplorer;
class ModelImportOptions : public IUserInterfaceElement
{
  public:
    ModelImportOptions(std::filesystem::path* modelPath, std::filesystem::path& saveToPath);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    std::filesystem::path*         m_path;
    std::filesystem::path&         m_saveToPath;
    ApplicationCore::ImportOptions m_options;
};

}  // namespace VEditor

#endif  //MODELIMPORTOPTIONS_HPP
