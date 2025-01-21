//
// Created by wpsimon09 on 28/12/24.
//

#include "SceneView.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Editor/Views/DetailsPanel/DetailsPanel.hpp"

namespace VEditor {
    SceneView::SceneView(const ApplicationCore::Scene& scene): m_scene(scene)
    {
        auto detailsPnel = std::make_unique<VEditor::DetailsPanel>();
        m_uiChildren.emplace_back(std::move(detailsPnel));

        m_detailsPanale = static_cast<DetailsPanel*>(m_uiChildren.back().get());

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
                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Delete", "DEL"))
                    {
                        if (m_selectedSceneNode)
                        {
                            m_scene.RemoveNode(m_selectedSceneNode->GetParent(), m_selectedSceneNode);
                        }
                    }

                    ImGui::EndPopup();
                }
                ImGui::EndChild();
        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void SceneView::CreateTreeView(std::shared_ptr<ApplicationCore::SceneNode> sceneNode)
    {
        if (!sceneNode)
            return;


        //===================================
        // SET NODE LABEL
        //===================================
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

        //Todo: ADD IS OPEN BOOLEAN TO THE SCENE NODE CLASS AND I WILL CHECK IF IT IS SELECTED FROM THERE, ROOT NODE WILL ALLWAYS BE SELECTED AND THERE FORE I HAVE ACCESS TO AT
        // LEAST TOP NODES OF THE HIERARCHY
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
                                      | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                      | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
                                      | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

        std::string visibilityButtonLabel;

        if (sceneNode->GetIsVisible()) visibilityButtonLabel = std::string(ICON_FA_EYE) + "##" + std::string(sceneNode->GetName());
        else                           visibilityButtonLabel = std::string(ICON_FA_EYE_SLASH) + "##" + std::string(sceneNode->GetName());

        auto oldFontSize = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale*= 0.4f;
        if (ImGui::Button(visibilityButtonLabel.c_str()))
        {
            if (sceneNode->GetIsVisible()) {
                sceneNode->Setvisibility(false);
            } else {
                sceneNode->Setvisibility(true);
            }
        }
        ImGui::GetFont()->Scale = oldFontSize;
        ImGui::SameLine();

        if (sceneNode->GetisSelectedFromWorld())
        {
            ImGui::SetNextItemOpen(true);
            m_selectedSceneNode = sceneNode;
            m_detailsPanale->SetSelectedNode(m_selectedSceneNode);
        }
        bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), nodeFlags);
        {

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
} // VEditor