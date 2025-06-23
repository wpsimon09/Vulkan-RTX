//
// Created by wpsimon09 on 28/12/24.
//

#include "SceneView.hpp"

#include "Application/AssetsManger/Utils/VTextureAsset.hpp"

#include <imgui.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Editor/Views/DetailsPanel/DetailsPanel.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace VEditor {
SceneView::SceneView(ApplicationCore::Scene& scene)
    : m_scene(scene)
{
    auto detailsPnel = std::make_unique<VEditor::DetailsPanel>(scene.GetAssetsManager());
    m_uiChildren.emplace_back(std::move(detailsPnel));

    m_detailsPanale = static_cast<DetailsPanel*>(m_uiChildren.back().get());
}

void SceneView::Resize(int newWidth, int newHeight) {}

void SceneView::Render()
{
    if(!m_detailsPanale->isSelectedSceneNodeSame(m_scene.GetSelectedSceneNode()))
        m_detailsPanale->SetSelectedNode(m_scene.GetSelectedSceneNode());

    ImGui::Begin(ICON_FA_MOUNTAIN " Scene graph", &m_isOpen,
                 ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::SeparatorText("Scene");

    ImGui::SameLine();
    if(ImGui::Button(ICON_FA_LAYER_GROUP))
    {
        m_openSceneDataView = true;
    }


    ImGui::BeginChild("Scrolling");

    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> lightNodes;
    lightNodes.reserve(m_scene.GetRootNode()->GetChildrenByRef().size());

    for(auto& sceneNode : m_scene.GetRootNode()->GetChildrenByRef())
    {
        if(!sceneNode->IsLight())
        {
            CreateTreeView(sceneNode);
        }
        else
        {
            lightNodes.emplace_back(sceneNode);
        }
    }

    ImGui::Dummy(ImVec2(0, 5.f));
    ImGui::Separator();
    CreateSceneLightsList(lightNodes);

    if(ImGui::BeginPopupContextWindow())
    {
        if(ImGui::MenuItem(ICON_FA_TRASH_CAN " Delete", "DEL"))
        {
            if(m_scene.GetSelectedSceneNode())
            {
                m_scene.RemoveNode(m_scene.GetSelectedSceneNode()->GetParent(), m_scene.GetSelectedSceneNode());
            }
        }

        ImGui::EndPopup();
    }
    ImGui::EndChild();
    ImGui::End();


    //=================================
    // Renders window with scene data
    //================================
    if(m_openSceneDataView)
    {
        RenderSceneDataView();
    }


    IUserInterfaceElement::Render();
}

void SceneView::CreateSceneLightsList(std::vector<std::shared_ptr<ApplicationCore::SceneNode>>& sceneLights)
{
    if(ImGui::TreeNodeEx(ICON_FA_LIGHTBULB " Lights", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))
    {
        for(auto& sceneNode : sceneLights)
        {
            std::string visibilityButtonLabel;
            if(sceneNode->GetIsVisible())
                visibilityButtonLabel = std::string(ICON_FA_EYE) + "##" + std::string(sceneNode->GetName());
            else
                visibilityButtonLabel = std::string(ICON_FA_EYE_SLASH) + "##" + std::string(sceneNode->GetName());

            if(ImGui::Button(visibilityButtonLabel.c_str()))
            {
                if(sceneNode->GetIsVisible())
                {
                    sceneNode->SetVisibility(false);
                }
                else
                {
                    sceneNode->SetVisibility(true);
                }
            }
            ImGui::SameLine();
            bool isSelected = false;
            if(m_scene.GetSelectedSceneNode())
            {
                if(m_scene.GetSelectedSceneNode() == sceneNode)
                {
                    isSelected = true;
                }
                else
                {
                    isSelected = false;
                }
            }
            if(ImGui::TreeNodeEx(GenerateNodeLabel(sceneNode).c_str(),
                                 ImGuiTreeNodeFlags_Leaf | (isSelected ? ImGuiTreeNodeFlags_Selected : 0)))
            {
                if(ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    m_scene.GetRootNode()->Deselect();
                    sceneNode->Select();
                    m_scene.SetSelectedSceneNode(sceneNode);
                    m_detailsPanale->SetSelectedNode(m_scene.GetSelectedSceneNode());
                }

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}


void SceneView::CreateTreeView(std::shared_ptr<ApplicationCore::SceneNode> sceneNode)
{
    if(!sceneNode)
        return;

    std::string nodeLabel = GenerateNodeLabel(sceneNode);

    //=================================================
    // CONSTRUCTING THE NODE
    //=================================================
    bool isLeaf = sceneNode->GetChildrenByRef().size() <= 0;

    //===========================================================
    // CHECKING IF CURRENT NODE IS SELECTED TO SET PROPER STYLING
    //===========================================================
    bool isSelected;

    if(m_scene.GetSelectedSceneNode())
    {
        if(m_scene.GetSelectedSceneNode() == sceneNode)
        {
            isSelected = true;
        }
        else
        {
            isSelected = false;
        }
    }


    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                   | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0) | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

    //===================================================
    // VISIBILITY SETTINGS
    //===================================================
    std::string visibilityButtonLabel;
    if(sceneNode->GetIsVisible())
        visibilityButtonLabel = std::string(ICON_FA_EYE) + "##" + std::string(sceneNode->GetName());
    else
        visibilityButtonLabel = std::string(ICON_FA_EYE_SLASH) + "##" + std::string(sceneNode->GetName());

    if(ImGui::Button(visibilityButtonLabel.c_str()))
    {
        if(sceneNode->GetIsVisible())
        {
            sceneNode->SetVisibility(false);
        }
        else
        {
            sceneNode->SetVisibility(true);
        }
    }
    ImGui::SameLine();

    //======================================================
    // SELECTION LOGIC
    //======================================================
    ImGui::SetNextItemOpen(sceneNode->IsOpen());
    if(!sceneNode->IsLight())
    {
        bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), nodeFlags);
        {

            if(ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                m_scene.GetRootNode()->Deselect();
                sceneNode->Select();
                m_scene.SetSelectedSceneNode(sceneNode);
                m_detailsPanale->SetSelectedNode(m_scene.GetSelectedSceneNode());
            }

            if(nodeOpen)
            {
                sceneNode->SetExpansionState(true);
                for(auto& child : sceneNode->GetChildrenByRef())
                {
                    CreateTreeView(child);
                }
                ImGui::TreePop();
            }
            else
            {
                sceneNode->SetExpansionState(false);
            }
        }
    }
}

std::string SceneView::GenerateNodeLabel(std::shared_ptr<ApplicationCore::SceneNode>& sceneNode)
{
    //==================================
    // GETTING INITIAL DATA
    //==================================
    std::string nodeLabel;
    switch(sceneNode->GetSceneNodeMetaData().nodeType)
    {
        case Node: {
            nodeLabel = std::string(ICON_FA_SQUARE_SHARE_NODES) + "  " + std::string(sceneNode->GetName());
            break;
        }
        case MeshNode: {
            nodeLabel = std::string(ICON_FA_BOX) + "  " + std::string(sceneNode->GetName());
            break;
        }
        case SkyBoxNode: {
            nodeLabel = std::string(ICON_FA_MOUNTAIN_SUN) + "  " + std::string(sceneNode->GetName());
            break;
        }
        case DirectionalLightNode: {
            nodeLabel = std::string(ICON_FA_SUN) + "  " + std::string(sceneNode->GetName());
            break;
        }
        case PointLightNode: {
            nodeLabel = std::string(ICON_FA_LIGHTBULB) + "  " + std::string(sceneNode->GetName());
            break;
        }
        case AreaLightNode: {
            nodeLabel = std::string(ICON_FA_SQUARE) + "  " + std::string(sceneNode->GetName());
            break;
        }
        case FogVolume: {
            nodeLabel = std::string(ICON_FA_SMOG) + "  " + std::string(sceneNode->GetName());
            break;
        }

        default: {
            nodeLabel = std::string(ICON_FA_QUESTION "Unknown scene node");
            break;
        }
    }
    return nodeLabel;
}

void SceneView::RenderSceneDataView()
{
    ImGui::Begin(ICON_FA_LAYER_GROUP " Scene data view ");

    ImGui::BeginTabBar("Scene data");

    {
        if(ImGui::BeginTabItem("Nodes"))
        {
            int i = 0;
            for(auto& n : m_scene.GetSceneData().nodes)
            {

                ImGui::Text("Index: %i", i);
                ImGui::SameLine();
                ImGui::Text(ICON_FA_CIRCLE_NODES " %s", n->GetName().c_str());
                i++;
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Meshes"))
        {
            int i = 0;
            for(auto& m : m_scene.GetSceneData().meshes)
            {

                ImGui::Text("Index: %i", i);
                ImGui::SameLine();
                ImGui::Text(ICON_FA_CUBE " %s", m->GetName().c_str());
                i++;
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials"))
        {
            int i = 0;
            for(auto& m : m_scene.GetSceneData().pbrMaterials)
            {

                ImGui::Text("%i", i);
                ImGui::SameLine();
                //ImGui::Text(ICON_FA_CIRCLE_HALF_STROKE " %s", m->GetMaterialName().c_str());
                std::string treeNodeLabel = ICON_FA_CIRCLE_HALF_STROKE  " Material##"+ std::to_string(i);
                if(ImGui::TreeNodeEx(treeNodeLabel.c_str()))
                {
                    ImGui::Text("Diffuse index %i", m->features.albedo);
                    ImGui::Text("Normal index %i", m->features.normalTextureIdx);
                    ImGui::Text("Arm index %i", m->features.armTextureIdx);
                    ImGui::Text("Emissive %i", m->features.emissiveTextureIdx);

                    ImGui::Separator();

                    ImGui::Text("Diffuce val") ; ImGui::SameLine() ;
                    ImVec4 col;
                    col.x = m->values.albedo.x;
                    col.y = m->values.albedo.y;
                    col.z = m->values.albedo.z;
                    col.w = m->values.albedo.w;
                    ImGui::ColorButton("Diffuce colour", col, ImGuiColorEditFlags_NoInputs);

                    ImGui::Text("Roughness %f", m->values.roughness);
                    ImGui::Text("Meallnes %f", m->values.metalness);
                    ImGui::Text("Ao %f", m->values.ao);

                    col.x = m->values.emissive_strength.x;
                    col.y = m->values.emissive_strength.y;
                    col.z = m->values.emissive_strength.z;
                    ImGui::ColorButton("Emissive colour", col, ImGuiColorEditFlags_NoInputs);

                    ImGui::TreePop();
                }
                i++;
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Textures"))
        {
            int i = 0;
            for(auto& t : m_scene.GetSceneData().textures)
            {

                ImGui::Text("Index: %i", i);
                ImGui::SameLine();
                ImGui::Text(ICON_FA_FILE_IMAGE " %s", t->GetName().c_str());
                i++;
            }
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();

    if(ImGui::Button("Close"))
    {
        m_openSceneDataView = false;
    }

    ImGui::End();
}

}  // namespace VEditor