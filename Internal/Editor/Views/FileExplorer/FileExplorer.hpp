//
// Created by wpsimon09 on 31/12/24.
//

#ifndef FILEEXPLORER_HPP
#define FILEEXPLORER_HPP
#include <filesystem>

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {
class ModelImportOptions;
}

namespace ApplicationCore {
class GLTFLoader;
}

namespace ApplicationCore {
class Scene;
}

namespace VEditor {

class FileExplorer : public IUserInterfaceElement
{
  public:
    explicit FileExplorer(const ApplicationCore::GLTFLoader& gltfLoader);
    std::filesystem::path* OpenForSceneImport();
    std::filesystem::path* OpenForMaterialImport();

    std::filesystem::path* GetPath() { return &m_filePath; };
    std::filesystem::path  GetPathCpy() { return m_filePath; };

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    const ApplicationCore::GLTFLoader&           m_gltfLoader;
    std::filesystem::path                        m_filePath;
    std::unique_ptr<VEditor::ModelImportOptions> m_importOptions;
};

class FileExplorer2 : public IUserInterfaceElement
{
  public:
    explicit FileExplorer2();
};

}  // namespace VEditor

#endif  //FILEEXPLORER_HPP
