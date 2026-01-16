//
// Created by simon on 15/01/2026.
//

#ifndef VULKAN_RTX_ASSETSBROWSERDRAWUTILS_HPP
#define VULKAN_RTX_ASSETSBROWSERDRAWUTILS_HPP
#include "IconsFontAwesome6.h"
#include "imgui.h"

#include <filesystem>
#include <vector>

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
    float IconSize    = 64.0f;
    float IconSpacing = 12.0f;
    int   Columns     = 1;

    int SelectedIndex = -1;

    AssetsBrowserDrawData() {}

    void UpdateLayout(float avail_width)
    {
        Columns = (int)(avail_width / (IconSize + IconSpacing));
        if(Columns < 1)
            Columns = 1;
    }


    void DrawGrid(const std::vector<DirectoryItem>& directoryItems)
    {
        ImDrawList* draw_list   = ImGui::GetWindowDrawList();
        float       avail_width = ImGui::GetContentRegionAvail().x;

        UpdateLayout(avail_width);

        ImVec2 start = ImGui::GetCursorScreenPos();

        for(int i = 0; i < directoryItems.size(); i++)
        {
            int col = i % Columns;
            int row = i / Columns;

            ImVec2 pos = {start.x + col * (IconSize + IconSpacing), start.y + row * (IconSize + IconSpacing)};

            ImGui::SetCursorScreenPos(pos);
            ImGui::PushID((int)directoryItems[i].id);

            ImGui::InvisibleButton("item", ImVec2(IconSize, IconSize));

            bool hovered = ImGui::IsItemHovered();
            bool clicked = ImGui::IsItemClicked();

            if(clicked)
                SelectedIndex = i;

            ImU32 bg_col = (SelectedIndex == i) ? IM_COL32(120, 140, 255, 255) :
                           hovered              ? IM_COL32(80, 80, 80, 255) :
                                                  IM_COL32(50, 50, 50, 255);

            draw_list->AddRect(pos, ImVec2(pos.x + IconSize, pos.y + IconSize), bg_col, 6.0f);

            const char* icon      = directoryItems[i].icon;
            ImVec2      icon_size = ImVec2(IconSize - 10, IconSize - 10);

            ImVec2 icon_pos = {pos.x + (IconSize - icon_size.x) * 0.5f, pos.y + (IconSize - icon_size.y) * 0.5f};

            draw_list->AddText(icon_pos, IM_COL32(255, 255, 255, 255), icon);

            char label[16];
            sprintf(label, "%s", directoryItems[i].name.c_str());
            draw_list->AddText(ImVec2(pos.x, pos.y + IconSize + 6), IM_COL32_WHITE, label);

            ImGui::PopID();
        }

        int rows = (directoryItems.size() + Columns - 1) / Columns;
        ImGui::Dummy(ImVec2(1.0f, rows * (IconSize + IconSpacing)));
    }
};

#endif  //VULKAN_RTX_ASSETSBROWSERDRAWUTILS_HPP
