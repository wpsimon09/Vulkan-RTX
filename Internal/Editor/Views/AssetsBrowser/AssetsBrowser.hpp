//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_ASSETSBROWSER_HPP
#define VULKAN_RTX_ASSETSBROWSER_HPP
#include "AssetsBrowserDrawUtils.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

#include <filesystem>

namespace ApplicationCore {
class Project;
}
namespace VEditor {

constexpr char* POP_UP_NEW_FOLDER_NAME       = "PopUpNewFolder";
constexpr char* POP_UP_ASSETS_PANEL_SETTINGS = "PopUpAssetPannelSettings";

class AssetsBrowser : public IUserInterfaceElement
{
  public:
    AssetsBrowser(ApplicationCore::Project& project);
    void RenderActions();
    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    ApplicationCore::Project& m_project;

    std::filesystem::path      m_previousPath;
    std::filesystem::path      m_currentPath;
    std::vector<DirectoryItem> m_items;
    std::filesystem::path      m_pathToDelete = "";
    AssetsBrowserDrawData      m_assetsBrowserGridDrawData;

    char newFolderName[100] = "New folder";
    bool m_deleteRequested  = false;
    bool m_createRequested  = true;
    bool m_refreshRequested = true;

  private:
    std::vector<DirectoryItem> ReadContentsOf(std::filesystem::path path);
    void                       RenderDirectoryTree(const std::filesystem::path& directory);
    void                       RenderCreateNewFolder();
    void                       RenderAssetsPanelSettings();
};

}  // namespace VEditor

#endif  //VULKAN_RTX_ASSETSBROWSER_HPP
