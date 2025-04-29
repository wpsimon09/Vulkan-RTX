//
// Created by wpsimon09 on 26/02/25.
//

#include "ModelImportOptions.hpp"

#include <future>
#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

namespace VEditor {

ModelImportOptions::ModelImportOptions(std::filesystem::path*             path,
                                       const ApplicationCore::GLTFLoader& gltfLoader,
                                       ApplicationCore::Scene&      scene)
    : m_gltfLoader(gltfLoader)
    , m_scene(scene)
    , m_path(path)
    , m_options{}
{
}

void ModelImportOptions::Render()
{
    if(ImGui::BeginPopupModal(ICON_FA_TOOLBOX " Import options"))
    {
        auto path = "Model name: " + m_path->string().substr(m_path->string().rfind('/', m_path->string().size() - 1));
        ImGui::Text(path.c_str());

        ImGui::Checkbox("Import materials", &m_options.importMaterials);
        ImGui::Checkbox("Import only materials", &m_options.importOnlyMaterials);
        ImGui::DragFloat("Uniform scale", &m_options.uniformScale, 1.0f, 1.0f);


        if(ImGui::Button("Import"))
        {
            std::async([this]() { return m_gltfLoader.LoadGLTFScene(m_scene, *m_path, m_options); });

            ImGui::OpenPopup("Importing");

            if(ImGui::BeginPopupModal("Importing"))
            {
                ImGui::Text("Importing, please wait");

                ImGui::EndPopup();
            }
            ImGui::CloseCurrentPopup();
        };

        ImGui::SameLine();


        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));

        if(ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(3);

        ImGui::EndPopup();
    }
    IUserInterfaceElement::Render();
}

void ModelImportOptions::Resize(int newWidth, int newHeight) {}

void ModelImportOptions::Update()
{
    IUserInterfaceElement::Update();
}
}  // namespace VEditor