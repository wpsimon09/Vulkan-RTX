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

ModelImportOptions::ModelImportOptions(const ApplicationCore::GLTFLoader& gltfLoader,
                                       std::filesystem::path*             modelPath,
                                       std::filesystem::path&             saveToPath)
    : m_path(modelPath)
    , m_saveToPath(saveToPath)
    , m_options{}
    , m_gltfLoader(gltfLoader)
{
}
void ModelImportOptions::Render()
{
    if(ImGui::BeginPopupModal(ICON_FA_TOOLBOX " Import options"))
    {
        //auto path = "Model name: " + m_path->string().substr(m_path->string().rfind('/', m_path->string().size() - 1));
        //ImGui::Text(path.c_str());
        ImGui::Text("Saving to %s", m_saveToPath.c_str());

        ImGui::Checkbox("Import materials", &m_options.importMaterials);
        ImGui::Checkbox("Import only materials", &m_options.importOnlyMaterials);
        ImGui::DragFloat("Uniform scale", &m_options.uniformScale, 1.0f, 1.0f);


        if(ImGui::Button("Import"))
        {
            m_gltfLoader.LoadGLTFScene(m_saveToPath, *m_path, m_options);
            ImGui::CloseCurrentPopup();
        }

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