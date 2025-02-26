//
// Created by wpsimon09 on 26/02/25.
//

#include "ModelImportOptions.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

namespace VEditor {

    ModelImportOptions::ModelImportOptions(std::filesystem::path* path, const ApplicationCore::GLTFLoader& gltfLoader, const ApplicationCore::Scene& scene): m_gltfLoader(gltfLoader), m_scene(scene), m_path(path)
    {
    }

    void ModelImportOptions::Render()
    {
        if (ImGui::BeginPopupModal(ICON_FA_TOOLBOX" Import options"))
        {
            auto path = "Mode: "+m_path->string().rfind('/', m_path->string().size()-1);
            ImGui::Text(path);

            ImGui::SameLine();

            if (ImGui::Button("Import")){
                const auto loadedNodes = m_gltfLoader.LoadGLTFScene(*m_path);
                for (const auto& scene_node : loadedNodes)
                {
                    m_scene.AddNode(scene_node);
                }
                ImGui::CloseCurrentPopup();
            };

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        IUserInterfaceElement::Render();

    }

    void ModelImportOptions::Resize(int newWidth, int newHeight)
    {
    }

    void ModelImportOptions::Update()
    {
        IUserInterfaceElement::Update();
    }
} // VEditor