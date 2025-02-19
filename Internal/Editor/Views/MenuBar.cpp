//
// Created by wpsimon09 on 31/12/24.
//

#include "MenuBar.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/GLTFExporter/GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include "Editor/Editor.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "FileExplorer/FileExplorer.hpp"

namespace VEditor {
    MenuBar::MenuBar(Editor* editor): m_editor(editor)
    {
        auto fileExplorer = std::make_unique<FileExplorer>(editor->m_uiContext.GetClient().GetGLTFLoader(),editor->m_uiContext.GetScene());
        m_uiChildren.emplace_back(std::move(fileExplorer));
        m_fileExplorer = dynamic_cast<FileExplorer*>(m_uiChildren.back().get());
    }

    void MenuBar::Resize(int newWidth, int newHeight)
    {
    }

    void MenuBar::Render()
    {
        //TODO: disable window border
        ImGui::BeginMainMenuBar();
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem(ICON_FA_DOWNLOAD" Import"))
                {
                    OnImportSelect();
                }
                if (ImGui::MenuItem(ICON_FA_FILE" Save"))
                {
                    Utils::Logger::LogInfo("Saving scene...");
                    m_editor->m_uiContext.GetClient().GetGLTFExporter().ExportScene(
                    "cache",
                        m_editor->m_uiContext.GetScene(),
                           m_editor->m_uiContext.GetClient().GetAssetsManager()
                        );
                }
                ImGui::EndMenu();
            }
        ImGui::EndMainMenuBar();
        IUserInterfaceElement::Render();
    }

    void MenuBar::Update()
    {

        IUserInterfaceElement::Update();
    }

    void MenuBar::OnImportSelect()
    {
        Utils::Logger::LogInfo("Importing file...");
        m_isFileDialoOpen = true;
        m_fileExplorer->OpenForSceneImport();

    }
} // VEditor