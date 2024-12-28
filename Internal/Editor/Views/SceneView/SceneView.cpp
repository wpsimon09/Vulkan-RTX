//
// Created by wpsimon09 on 28/12/24.
//

#include "SceneView.hpp"

#include <imgui.h>

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
        ImGui::Begin("Scene graph");
            ImGui::SeparatorText("Scene");
            CreateTreeView(m_scene.GetRootNode());
        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void SceneView::CreateTreeView(std::shared_ptr<ApplicationCore::SceneNode>& sceneNode)
    {
        if (!sceneNode)
            return;

        // Get the name of the node or use a default name
        std::string nodeName = sceneNode->GetName().empty() ? "Unnamed Node" : std::string(sceneNode->GetName());

        // Create a tree node for the current scene node
        if (ImGui::TreeNode(nodeName.c_str()))
        {
            // Display information about the current node
            ImGui::Text("Has Mesh: %s", sceneNode->HasMesh() ? "Yes" : "No");

            // Recursively create tree nodes for the children
            for (auto& child : sceneNode->GetChildren())
            {
                CreateTreeView(child);
            }

            // End the current tree node
            ImGui::TreePop();
        }
    }
} // VEditor