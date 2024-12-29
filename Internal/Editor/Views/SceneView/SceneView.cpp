//
// Created by wpsimon09 on 28/12/24.
//

#include "SceneView.hpp"

#include <imgui.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"

namespace VEditor {
    SceneView::SceneView(const ApplicationCore::Scene& scene): m_scene(scene)
    {

    }

    void SceneView::Resize(int newWidth, int newHeight)
    {
    }

    void SceneView::Render()
    {
        ImGui::Begin("Scene graph",&m_isOpen, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

            ImGui::SeparatorText("Scene");

                ImGui::BeginChild("Scrolling");
                CreateTreeView(m_scene.GetRootNode());
            ImGui::EndChild();
        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void SceneView::CreateTreeView(std::shared_ptr<ApplicationCore::SceneNode>& sceneNode)
    {
        if (!sceneNode)
            return;

        if (ImGui::TreeNode(sceneNode->GetName().data()))
        {
            for (auto& child : sceneNode->GetChildren2())
            {
                CreateTreeView(child);
            }
            if (sceneNode->HasMesh())
            {
                ImGui::Selectable(ICON_FA_SEARCH );
            }
            ImGui::TreePop();
        }
    }
} // VEditor