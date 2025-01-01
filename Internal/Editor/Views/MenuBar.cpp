//
// Created by wpsimon09 on 31/12/24.
//

#include "MenuBar.hpp"

#include <imgui.h>

#include "Application/Logger/Logger.hpp"
#include "Editor/Editor.hpp"
#include "FileExplorer/FileExplorer.hpp"

namespace VEditor {
    MenuBar::MenuBar(Editor* editor): m_editor(editor)
    {
        auto fileExplorer = std::make_unique<FileExplorer>();
        m_uiChildren.emplace_back(std::move(fileExplorer));
        m_fileExplorer = dynamic_cast<FileExplorer*>(m_uiChildren.back().get());
    }

    void MenuBar::Resize(int newWidth, int newHeight)
    {
    }

    void MenuBar::Render()
    {
        ImGui::BeginMainMenuBar();
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import"))
                {
                    OnImportSelect();
                }
                if (ImGui::MenuItem("Save"))
                {

                }
                ImGui::EndMenu();
            }

        ImGui::EndMainMenuBar();
        IUserInterfaceElement::Render();
    }

    void MenuBar::Update()
    {

        if (!m_fileExplorer->GetPath()->empty() && m_isFileDialoOpen)
        {
            m_editor->m_filePath = m_fileExplorer->GetPathCpy();
            // cler the selected path so that new path can be selected
            m_fileExplorer->GetPath()->clear();
            m_isFileDialoOpen = false;
        }
        IUserInterfaceElement::Update();
    }

    void MenuBar::OnImportSelect()
    {
        Utils::Logger::LogInfo("Importing file...");
        m_isFileDialoOpen = true;
        m_fileExplorer->Open();

    }
} // VEditor