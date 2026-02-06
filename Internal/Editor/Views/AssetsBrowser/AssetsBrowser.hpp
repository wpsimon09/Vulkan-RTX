//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_ASSETSBROWSER_HPP
#define VULKAN_RTX_ASSETSBROWSER_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"
#include "AssetsBrowserDrawUtils.hpp"

#include <filesystem>

namespace ApplicationCore {
class RuntimeAssetsManager;
struct ImportOptions;
class GLTFLoader;
class Project;
}  // namespace ApplicationCore
namespace VEditor {
class FileExplorer;

constexpr char* POP_UP_NEW_FOLDER_NAME       = "PopUpNewFolder";
constexpr char* POP_UP_ASSETS_PANEL_SETTINGS = "PopUpAssetPannelSettings";

class AssetsBrowser : public IUserInterfaceElement
{
  public:
    AssetsBrowser(ApplicationCore::GLTFLoader&           gltfLoader,
                  ApplicationCore::Project&              project,
                  ApplicationCore::RuntimeAssetsManager& runtimeAssetsManager);
    void RenderActions();
    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    ApplicationCore::Project& m_project;

    std::filesystem::path        m_previousPath;
    std::filesystem::path        m_currentPath;
    std::vector<DirectoryItem>   m_items;
    std::filesystem::path        m_pathToDelete = "";
    AssetsBrowserDrawData        m_assetsBrowserGridDrawData;
    VEditor::FileExplorer*       m_fileExplorer;
    std::filesystem::path*       m_importPath = nullptr;
    std::optional<DirectoryItem> m_selectedItem;

    ApplicationCore::VMaterial m_previewMaterial;
    ApplicationCore::VTexture  m_previewTexture;
    ApplicationCore::VMesh     m_previewMesh;

    ApplicationCore::RuntimeAssetsManager& m_runtimeAssetsManager;

    char                         newFolderName[100] = "New folder";
    bool                         m_deleteRequested  = false;
    bool                         m_createRequested  = true;
    bool                         m_refreshRequested = true;
    ApplicationCore::GLTFLoader& m_gltfLoader;

  private:
    std::vector<DirectoryItem> ReadContentsOf(std::filesystem::path path);
    void                       RenderDirectoryTree(const std::filesystem::path& directory);
    void                       RenderCreateNewFolder();
    void                       RenderAssetsPanelSettings();
    void                       RenderInspectPopUp();
};

}  // namespace VEditor

#endif  //VULKAN_RTX_ASSETSBROWSER_HPP
