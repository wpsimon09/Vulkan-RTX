//
// Created by wpsimon09 on 31/12/24.
//

#include "FileExplorer.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui.h"

namespace VEditor {
    FileExplorer::FileExplorer()
    {

    }

    std::filesystem::path* FileExplorer::Open()
    {

        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config);
        ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey");

        return nullptr;
    }

    void FileExplorer::Render()
    {
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                m_filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            }

            // close
            ImGuiFileDialog::Instance()->Close();

        }

        IUserInterfaceElement::Render();
    }

    void FileExplorer::Resize(int newWidth, int newHeight)
    {
    }

    void FileExplorer::Update()
    {

        IUserInterfaceElement::Update();
    }
} // VEditor