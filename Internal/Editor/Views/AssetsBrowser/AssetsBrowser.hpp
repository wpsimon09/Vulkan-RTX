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

class AssetsBrowser : public IUserInterfaceElement
{
  public:
    AssetsBrowser(ApplicationCore::Project& project);
    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    ApplicationCore::Project& m_project;

    std::filesystem::path   m_previousPath;
    std::filesystem::path   m_currentPath;
    ImVector<DirectoryItem> m_items;
};

}  // namespace VEditor

#endif  //VULKAN_RTX_ASSETSBROWSER_HPP
