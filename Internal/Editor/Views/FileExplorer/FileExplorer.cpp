//
// Created by wpsimon09 on 31/12/24.
//

#include "FileExplorer.hpp"

#include <IconsFontAwesome6.h>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Editor/Views/Pop-Ups/ModelImportOptions/ModelImportOptions.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui.h"

namespace VEditor {

    FileExplorer::FileExplorer(const ApplicationCore::GLTFLoader& gltfLoader, const ApplicationCore::Scene& scene): m_scene(scene), m_gltfLoader(gltfLoader)
    {
        m_uiChildren.emplace_back(std::make_unique<VEditor::ModelImportOptions>(&m_filePath, gltfLoader, scene));
    }

    std::filesystem::path* FileExplorer::OpenForSceneImport()
    {

        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("SelectModelKey", "Choose Model file", ".glb,.gltf,.hdr", config);
        ImGuiFileDialog::Instance()->Display("SelectModelKey");

        return nullptr;
    }

    std::filesystem::path* FileExplorer::OpenForMaterialImport()
    {

        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("SelectMaterialKey", "Choose File material file", ".cpp,.h,.hpp", config);
        ImGuiFileDialog::Instance()->Display("SelectMaterialKey");

        return nullptr;
    }

    void FileExplorer::Render()
    {
        if (ImGuiFileDialog::Instance()->Display("SelectModelKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                m_filePath = ImGuiFileDialog::Instance()->GetFilePathName();

                if (m_filePath.extension() == ".glb" || m_filePath.extension() == ".gltf")
                {
                    ImGui::OpenPopup(ICON_FA_TOOLBOX" Import options");
                }else
                {
                    m_scene.GetAssetsManager().AddSkyBoxMaterial(m_filePath);
                }
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SelectMaterialKey")) {
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