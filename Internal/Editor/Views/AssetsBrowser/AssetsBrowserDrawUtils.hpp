//
// Created by simon on 15/01/2026.
//

#ifndef VULKAN_RTX_ASSETSBROWSERDRAWUTILS_HPP
#define VULKAN_RTX_ASSETSBROWSERDRAWUTILS_HPP
#include "AssetsBrowser.hpp"
#include "AssetsBrowser.hpp"
#include "AssetsBrowser.hpp"
#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/Logger/Logger.hpp"
#include "Application/Project/Project.hpp"
#include "Editor/Editor.hpp"

#include <filesystem>
#include <vector>


namespace VEditor {
static auto DRAG_DROP_PAYLOAD_MESH     = "DragDropPayloadMesh";
static auto DRAG_DROP_PAYLOAD_MATERIAL = "DragDropPayloadMaterial";

struct DirectoryItem
{
    bool                  isFile{true};
    std::string           name{""};
    const char*           icon{ICON_FA_QUESTION};
    std::filesystem::path path{};
    int                   id;
    DirectoryItem() = default;

    // Copy constructor
    DirectoryItem(const DirectoryItem& other)
        : isFile(other.isFile)
        , name(other.name)
        , icon(other.icon)
        , path(other.path)
        , id(other.id)
    {
    }

    DirectoryItem& operator=(const DirectoryItem& other)
    {
        if(this != &other)
        {
            isFile = other.isFile;
            name   = other.name;
            icon   = other.icon;
            path   = other.path;
            id     = other.id;
        }
        return *this;
    }
};

struct AssetsBrowserDrawData
{
    ApplicationCore::EditorConfig& editorConf;
    int                            SelectedIndex = -1;
    std::filesystem::path&         m_currentPath;

    AssetsBrowserDrawData(ApplicationCore::EditorConfig& editorConfig, std::filesystem::path& currentPath)
        : editorConf(editorConfig)
        , m_currentPath(currentPath)
    {
    }

    void UpdateLayout(float avail_width)
    {
        editorConf.Columns = (int)(avail_width / (editorConf.TileSize + editorConf.IconSpacing));
        if(editorConf.Columns < 1)
            editorConf.Columns = 1;
    }


    void DrawGrid(const std::vector<DirectoryItem>& directoryItems)
    {
        ImDrawList* draw_list   = ImGui::GetWindowDrawList();
        float       avail_width = ImGui::GetContentRegionAvail().x;

        UpdateLayout(avail_width);

        ImVec2 start = ImGui::GetCursorScreenPos();

        for(int i = 0; i < directoryItems.size(); i++)
        {
            int col = i % editorConf.Columns;
            int row = i / editorConf.Columns;

            ImVec2 pos = {start.x + col * (editorConf.TileSize + editorConf.IconSpacing),
                          start.y + row * (editorConf.TileSize + editorConf.IconSpacing)};

            ImGui::SetCursorScreenPos(pos);
            ImGui::PushID((int)directoryItems[i].id);

            ImGui::InvisibleButton("item", ImVec2(editorConf.TileSize, editorConf.TileSize));

            bool hovered       = ImGui::IsItemHovered();
            bool clicked       = ImGui::IsItemClicked();
            bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

            if(clicked)
                SelectedIndex = i;
            if(doubleClicked && !directoryItems[i].isFile)
            {
                m_currentPath = directoryItems[i].path;
            }

            ImU32 bg_col = (SelectedIndex == i) ? IM_COL32(120, 140, 255, 255) :
                           hovered              ? IM_COL32(80, 80, 80, 255) :
                                                  IM_COL32(50, 50, 50, 255);


            draw_list->AddRect(pos, ImVec2(pos.x + editorConf.TileSize, pos.y + editorConf.TileSize), bg_col, 6.0f);

            ImGui::PushFont(NULL, editorConf.AssetBrowserIconSize);
            const char* icon      = directoryItems[i].icon;
            ImVec2      icon_size = ImGui::CalcTextSize(icon);
            ImVec2      icon_pos  = {pos.x + (editorConf.TileSize - icon_size.x) * 0.5f,
                                     pos.y + (editorConf.TileSize - icon_size.y) * 0.5f};
            draw_list->AddText(icon_pos, IM_COL32(255, 255, 255, 255), icon);

            ImGui::PopFont();

            char label[16];
            sprintf(label, "%s", directoryItems[i].name.c_str());
            draw_list->AddText(ImVec2(pos.x, pos.y + editorConf.TileSize + 6), IM_COL32_WHITE, label);

            if(directoryItems[i].isFile)
            {

                if(ImGui::BeginDragDropSource())
                {
                    ImGui::PushFont(NULL, 40.0f);
                    ImGui::Text(ICON_FA_FILE);
                    ImGui::PopFont();
                    ImGui::Text(directoryItems[i].name.c_str());

                    ImGui::SetDragDropPayload(VEditor::DRAG_DROP_PAYLOAD_MATERIAL, directoryItems[i].name.c_str(),
                                              directoryItems[i].name.size() * sizeof(char));

                    ImGui::EndDragDropSource();
                }
            }

            ImGui::PopID();
        }

        int rows = (directoryItems.size() + editorConf.Columns - 1) / editorConf.Columns;
        ImGui::Dummy(ImVec2(1.0f, rows * (editorConf.TileSize + editorConf.IconSpacing)));
    }
};
}  // namespace VEditor

#endif  //VULKAN_RTX_ASSETSBROWSERDRAWUTILS_HPP
