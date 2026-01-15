//
// Created by simon on 14/01/2026.
//

#include "AssetsBrowser.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/Project/Project.hpp"

#include <filesystem>

namespace VEditor {

AssetsBrowser::AssetsBrowser(ApplicationCore::Project& project)
    : IUserInterfaceElement()
    , m_project(project)
    , m_currentPath(project.GetProjectPath())
{
}
void AssetsBrowser::Render()
{
    ImGui::Begin(ICON_FA_FOLDER " Project browser");

    //====================================================
    // Right panel with overview of the directory structure
    ImGui::BeginChild(ICON_FA_FOLDER_TREE " Project overview", ImVec2(ImGui::GetWindowWidth() / 4, 0), ImGuiChildFlags_Border);
    {
        for(auto& item : m_items)
        {
            auto label = item.icon + std::string(item.name);
            if(ImGui::Selectable(*item.icon + item.name, m_currentPath == item.path))
            {
                m_currentPath = item.path;
            }
        }
        ImGui::EndChild();
    }

    ImGui::SameLine();

    //=========================================
    // The actual assets browser
    ImGui::BeginChild(ICON_FA_MAGNIFYING_GLASS " Current folder ", ImVec2(ImGui::GetWindowWidth() - 10, 0), ImGuiChildFlags_Border);
    {
        ImGui::Text("Haloooo");

        ImGui::EndChild();
    }


    ImGui::End();
    IUserInterfaceElement::Render();
}
void AssetsBrowser::Resize(int newWidth, int newHeight) {}

void AssetsBrowser::Update()
{
    if(m_currentPath != m_previousPath)
    {
        m_previousPath = m_currentPath;
        m_items.clear();
        int id = 0;
        for(const auto& directory : std::filesystem::directory_iterator(m_currentPath))
        {
            DirectoryItem item{directory.is_regular_file(), directory.path().filename().c_str(),
                               directory.is_regular_file() ? ICON_FA_FILE : ICON_FA_FOLDER, directory, id};
            m_items.push_back(item);
            id++;
        }
    }
    IUserInterfaceElement::Update();
}
}  // namespace VEditor