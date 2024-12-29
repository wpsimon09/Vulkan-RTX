//
// Created by wpsimon09 on 28/12/24.
//

#include "SceneView.hpp"

#include <imgui.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

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
        ImGui::Begin(ICON_FA_ATOM" Scene graph",&m_isOpen, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

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

        std::string nodeLabel;
        if (!sceneNode->HasMesh())
        {
            nodeLabel = std::string(ICON_FA_SQUARE_SHARE_NODES) + "  " + std::string(sceneNode->GetName());
        }else
        {
            nodeLabel = std::string(ICON_FA_BOX) + "  " + std::string(sceneNode->GetName());
        }

        bool isSelected;
        if (m_selectedSceneNode)
            isSelected = (m_selectedSceneNode == sceneNode);
        else
            isSelected = false;

        bool isLeaf =  sceneNode->GetChildren2().size() <= 0;
        // Use ImGui::Selectable for making nodes selectable
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
                                      | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                      | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
                                      | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

        bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), nodeFlags);
        {

            if (ImGui::IsItemClicked())
            {
                m_selectedSceneNode = sceneNode; // Set the currently selected node
            }

            if (nodeOpen)
            {
                for (auto& child : sceneNode->GetChildren2())
                {
                    CreateTreeView(child);
                }
                ImGui::TreePop();
            }
        }
    }
} // VEditor