//
// Created by wpsimon09 on 28/12/24.
//

#include "SceneView.hpp"

#include <imgui.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Editor/Views/DetailsPanel/DetailsPanel.hpp"

namespace VEditor {
    SceneView::SceneView(ApplicationCore::Scene& scene): m_scene(scene)
    {
        auto detailsPnel = std::make_unique<VEditor::DetailsPanel>(scene.GetAssetsManager());
        m_uiChildren.emplace_back(std::move(detailsPnel));

        m_detailsPanale = static_cast<DetailsPanel*>(m_uiChildren.back().get());

    }

    void SceneView::Resize(int newWidth, int newHeight)
    {

    }

    void SceneView::Render()
    {
        if (!m_detailsPanale->isSelectedSceneNodeSame(m_scene.GetSelectedSceneNode()))
            m_detailsPanale->SetSelectedNode(m_scene.GetSelectedSceneNode());

        ImGui::Begin(ICON_FA_MOUNTAIN" Scene graph",&m_isOpen, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
            ImGui::SeparatorText("Scene");
                ImGui::BeginChild("Scrolling");
                for (auto& sceneNode : m_scene.GetRootNode()->GetChildrenByRef())
                {
                    CreateTreeView(sceneNode);
                }

                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Delete", "DEL") )
                    {
                        if (m_scene.GetSelectedSceneNode())
                        {
                            m_scene.RemoveNode(m_scene.GetSelectedSceneNode()->GetParent(), m_scene.GetSelectedSceneNode());
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

        //==================================
        // GETTING INITIAL DATA
        //==================================
        std::string nodeLabel;
        switch (sceneNode->GetSceneNodeMetaData().nodeType)
        {
            case Node:{ nodeLabel = std::string(ICON_FA_SQUARE_SHARE_NODES) + "  " + std::string(sceneNode->GetName());     break;   }
            case MeshNode:{ nodeLabel = std::string(ICON_FA_BOX) + "  " + std::string(sceneNode->GetName()); break;   }
            case DirectionalLightNode:{ nodeLabel = std::string(ICON_FA_SUN) + "  " + std::string(sceneNode->GetName());    break;   }
            case PointLightNode:{ nodeLabel = std::string(ICON_FA_LIGHTBULB) + "  " + std::string(sceneNode->GetName());    break;   }
            case AreaLightNode:{ nodeLabel = std::string(ICON_FA_SQUARE) + "  " + std::string(sceneNode->GetName());    break;   }
            default:{ nodeLabel = std::string(ICON_FA_QUESTION  "Unknown scene node");    break;   }

        }

        //=================================================
        // CONSTRUCTING THE NODE
        //=================================================
        bool isLeaf =  sceneNode->GetChildrenByRef().size() <= 0;

        //===========================================================
        // CHECKING IF CURRENT NODE IS SELECTED TO SET PROPER STYLING
        //===========================================================
        bool isSelected;

        if (m_scene.GetSelectedSceneNode())
        {
            if (m_scene.GetSelectedSceneNode() == sceneNode) {
                isSelected = true;
            }
            else{
                isSelected = false;
            }
        }


        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
                                      | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                      | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
                                      | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

        //===================================================
        // VISIBILITY SETTINGS
        //===================================================
        std::string visibilityButtonLabel;
        if (sceneNode->GetIsVisible()) visibilityButtonLabel = std::string(ICON_FA_EYE) + "##" + std::string(sceneNode->GetName());
        else                           visibilityButtonLabel = std::string(ICON_FA_EYE_SLASH) + "##" + std::string(sceneNode->GetName());

        auto oldFontSize = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale*= 0.4f;
        if (ImGui::Button(visibilityButtonLabel.c_str()))
        {
            if (sceneNode->GetIsVisible())
            {
                sceneNode->Setvisibility(false);
            }
            else
            {
                sceneNode->Setvisibility(true);
            }
        }
        ImGui::GetFont()->Scale = oldFontSize;
        ImGui::SameLine();

        //======================================================
        // SELECTION LOGIC
        //======================================================
        ImGui::SetNextItemOpen(sceneNode->IsOpen());


        bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), nodeFlags);
        {

            if (ImGui::IsItemClicked() ||
                ImGui::IsItemClicked(ImGuiMouseButton_Right)
                ){
                    m_scene.GetRootNode()->Deselect();
                    sceneNode->Select();
                    m_scene.SetSelectedSceneNode(sceneNode);
                    m_detailsPanale->SetSelectedNode(m_scene.GetSelectedSceneNode());
            }

            if (nodeOpen)
            {
                sceneNode->SetExpansionState(true);
                for (auto& child : sceneNode->GetChildrenByRef())
                {
                    CreateTreeView(child);
                }
                ImGui::TreePop();
            }else
            {
                sceneNode->SetExpansionState(false);
            }
        }
    }
} // VEditor