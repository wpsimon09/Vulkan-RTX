//
// Created by wpsimon09 on 31/12/24.
//

#include "FileExplorer.hpp"

#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "imgui/imgui.h"

namespace VEditor {

    FileExplorer::FileExplorer(const ApplicationCore::GLTFLoader& gltfLoader, const ApplicationCore::Scene& scene): m_scene(scene), m_gltfLoader(gltfLoader)
    {
    }

    std::filesystem::path* FileExplorer::OpenForSceneImport()
    {

        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("SelectModelKey", "Choose Model file", ".glb,.gltf", config);
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
                const auto loadedNodes = m_gltfLoader.LoadGLTFScene(m_filePath);
                for (auto &scene_node : loadedNodes)
                {
                    m_scene.AddNode(scene_node);
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