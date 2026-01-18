//
// Created by simon on 14/01/2026.
//

#include "AssetsBrowser.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Application/AssetsSystem/VMesh.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Project/Project.hpp"
#include "Application/VertexArray/VertexArray.hpp"

#include <filesystem>
#include <glm/ext/matrix_projection.hpp>

namespace VEditor {

AssetsBrowser::AssetsBrowser(ApplicationCore::Project& project)
    : IUserInterfaceElement()
    , m_project(project)
    , m_assetsBrowserGridDrawData(project.GetProjectConfig().editorConfig, m_currentPath)
{
}
void AssetsBrowser::RenderActions()
{

    bool disabledBackButton = m_currentPath.has_parent_path() && m_currentPath == m_project.GetProjectPath();
    ImGui::BeginDisabled(disabledBackButton);
    if(ImGui::Button(ICON_FA_CIRCLE_LEFT, ImVec2(ImGui::GetFontSize() + 10.0f, 0)))
    {
        m_currentPath = m_currentPath.parent_path();
    }
    ImGui::SameLine();
    ImGui::EndDisabled();

    if(ImGui::Button(ICON_FA_TOOLBOX " Options"))
    {
        ImGui::OpenPopup(POP_UP_ASSETS_PANEL_SETTINGS);
    }
    RenderAssetsPanelSettings();
    ImGui::SameLine();

    if(ImGui::Button(ICON_FA_REPLY))
    {
        m_refreshRequested = true;
    }

    ImGui::Text(m_currentPath.c_str());


    bool dragActive = ImGui::IsDragDropActive();

    if(dragActive)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.9f, 0.6f));
    }

    ImGui::Button("Drop here");

    if(dragActive)
        ImGui::PopStyleColor(2);

    if(ImGui::BeginDragDropTarget())
    {

        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(VEditor::DRAG_DROP_PAYLOAD_MATERIAL))
        {
            auto itemName = static_cast<char*>(payload->Data);
            Utils::Logger::LogInfoClient("Drag successfully: " + std::string(itemName));
        }
        ImGui::EndDragDropTarget();
    }

    if(ImGui::Button("Add mesh"))
    {
        std::vector<ApplicationCore::Vertex> test  = {{}, {}, {}};
        std::vector<uint32_t>                testi = {0, 1, 2, 3, 4, 5};
        ApplicationCore::VMesh               testMesh("test", m_currentPath, test, testi);
    }
}

void AssetsBrowser::Render()
{
    if(m_currentPath.empty())
    {
        m_currentPath = m_project.GetProjectPath();
    }

    ImGui::Begin(ICON_FA_FOLDER " Project browser");

    //====================================================
    // Right panel with overview of the directory structure
    ImGui::BeginChild(ICON_FA_FOLDER_TREE " Project overview", ImVec2(ImGui::GetContentRegionAvail().x / 4, 0), ImGuiChildFlags_Border);
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
    ImGui::BeginChild(ICON_FA_MAGNIFYING_GLASS " Current folder ", ImVec2(ImGui::GetContentRegionAvail().x, 0), ImGuiChildFlags_Border);
    {
        RenderActions();

        ImGui::Separator();

        m_assetsBrowserGridDrawData.DrawGrid(m_items);

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
    m_deleteRequested = false;

    if((m_currentPath != m_previousPath) || m_refreshRequested)
    {
        m_previousPath = m_currentPath;
        m_items.clear();
        m_items            = ReadContentsOf(m_currentPath);
        m_refreshRequested = false;
    }
    IUserInterfaceElement::Update();
}
std::vector<DirectoryItem> AssetsBrowser::ReadContentsOf(std::filesystem::path path)
{
    std::vector<DirectoryItem> result;
    int                        id = 0;
    for(const auto& directory : std::filesystem::directory_iterator(m_currentPath))
    {
        DirectoryItem item;
        item.path   = directory;
        item.icon   = directory.is_directory() ? ICON_FA_FOLDER : ICON_FA_FILE;
        item.id     = id;
        item.isFile = directory.is_regular_file();
        item.name   = directory.path().filename().string();

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
        else if(ImGui::IsItemClicked() && !isDirectory)
        {
            m_currentPath = path.parent_path();
        }

        if(ImGui::BeginPopupContextItem())
        {
            m_currentPath = path;
            if(ImGui::MenuItem(ICON_FA_TRASH " Delete folder") || ImGui::Shortcut(ImGuiKey_Delete))
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
void AssetsBrowser::RenderAssetsPanelSettings()
{
    if(ImGui::BeginPopupContextItem(POP_UP_ASSETS_PANEL_SETTINGS))
    {
        ImGui::DragFloat("Tile size", &m_assetsBrowserGridDrawData.editorConf.TileSize, 1.0, 0.1);
        ImGui::DragFloat("Icon spacing", &m_assetsBrowserGridDrawData.editorConf.IconSpacing, 0.1);
        ImGui::DragFloat("Icon font size", &m_assetsBrowserGridDrawData.editorConf.AssetBrowserIconSize, 0.5, 1.0);
        ImGui::DragInt("Max Columns", &m_assetsBrowserGridDrawData.editorConf.Columns, 1, 0);
        ImGui::EndPopup();
    }
}

}  // namespace VEditor