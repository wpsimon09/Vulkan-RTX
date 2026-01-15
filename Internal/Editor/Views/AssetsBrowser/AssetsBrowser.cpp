//
// Created by simon on 14/01/2026.
//

#include "AssetsBrowser.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/Project/Project.hpp"

#include <filesystem>
#include <glm/ext/matrix_projection.hpp>

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
        if(ImGui::Button(ICON_FA_CIRCLE_PLUS " Create folder "))
        {
            ImGui::OpenPopup(POP_UP_NEW_FOLDER_NAME);
        }
        RenderCreateNewFolder();

        ImGui::Separator();

        RenderDirectoryTree(m_project.GetProjectPath());
        ImGui::EndChild();
    }

    ImGui::SameLine();

    //=========================================
    // The actual assets browser
    ImGui::BeginChild(ICON_FA_MAGNIFYING_GLASS " Current folder ", ImVec2(ImGui::GetWindowWidth() - 10, 0), ImGuiChildFlags_Border);
    {
        ImGui::EndChild();
    }


    ImGui::End();
    IUserInterfaceElement::Render();
}
void AssetsBrowser::Resize(int newWidth, int newHeight) {}

void AssetsBrowser::Update()
{
    if(m_deleteRequested)
    {
        std::filesystem::remove_all(m_pathToDelete);
    }
    if(m_createRequested)
    {
    }
    m_createRequested = false;
    m_deleteRequested = false;

    if(m_currentPath != m_previousPath)
    {
        m_previousPath = m_currentPath;
        m_items.clear();
        m_items = ReadContentsOf(m_currentPath);
    }
    IUserInterfaceElement::Update();
}
std::vector<DirectoryItem> AssetsBrowser::ReadContentsOf(std::filesystem::path path)
{
    std::vector<DirectoryItem> result;
    int                        id = 0;
    for(const auto& directory : std::filesystem::directory_iterator(m_currentPath))
    {
        DirectoryItem item{directory.is_regular_file(), directory.path().filename().c_str(),
                           directory.is_regular_file() ? ICON_FA_FILE : ICON_FA_FOLDER, directory, id};
        result.push_back(item);
        id++;
    }
    return result;
}

void AssetsBrowser::RenderDirectoryTree(const std::filesystem::path& directory)
{
    for(const auto& entry : std::filesystem::directory_iterator(directory))
    {
        const auto& path        = entry.path();
        const bool  isDirectory = entry.is_directory();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                   | (isDirectory ? 0 : ImGuiTreeNodeFlags_Leaf)
                                   | (m_currentPath == path ? ImGuiTreeNodeFlags_Selected : 0);
        // persist the id
        ImGui::PushID(path.c_str());

        ImGuiID id     = ImGui::GetID(path.c_str());
        bool    isOpen = ImGui::GetStateStorage()->GetBool(id, false);

        const char* icon = (isDirectory && isOpen) ? ICON_FA_FOLDER_OPEN : (isDirectory ? ICON_FA_FOLDER : ICON_FA_FILE);

        std::string label = std::string(icon) + " " + path.filename().string();

        bool opened = ImGui::TreeNodeEx(label.c_str(), flags);


        if(ImGui::IsItemClicked() && isDirectory)
            m_currentPath = path;

        if(ImGui::BeginPopupContextItem())
        {
            m_currentPath = path;
            if(ImGui::MenuItem(ICON_FA_TRASH " Delete folder"))
            {
                m_deleteRequested = true;
                m_pathToDelete    = m_currentPath;
            }
            if(ImGui::Button(ICON_FA_CIRCLE_PLUS " Create"))
            {
                ImGui::OpenPopup(POP_UP_NEW_FOLDER_NAME);
            }
            RenderCreateNewFolder();
            ImGui::EndPopup();
        }
        if(opened)
        {
            if(isDirectory)
                RenderDirectoryTree(path);
            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}
void AssetsBrowser::RenderCreateNewFolder()
{
    if(ImGui::BeginPopupContextItem(POP_UP_NEW_FOLDER_NAME))
    {
        ImGui::InputText("Name", newFolderName, IM_ARRAYSIZE(newFolderName));
        if(ImGui::MenuItem("Create"))
        {
            std::filesystem::create_directory(m_currentPath / newFolderName);
            strcpy(newFolderName, "New folder name");
        }
        ImGui::EndPopup();
    }
}

}  // namespace VEditor