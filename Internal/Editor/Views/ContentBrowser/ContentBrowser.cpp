//
// Created by wpsimon09 on 03/03/25.
//

#include "ContentBrowser.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"

ContentBrowser::ContentBrowser(ApplicationCore::AssetsManager& assetManager, ApplicationCore::Scene& scene): IUserInterfaceElement(), m_assetManager(assetManager), m_scene(scene)
{
}

void ContentBrowser::Render()
{
    ImGui::Begin( ICON_FA_FOLDER_OPEN" ContentBrowser");

    ImGui::BeginTabBar("Items");
    {
        if (ImGui::BeginTabItem("Models"))
        {
            RenderModels();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Meshes"))
        {
            RenderMeshes();
            ImGui::EndTabItem();
        }


    }
    ImGui::EndTabBar();


    ImGui::End();
    IUserInterfaceElement::Render();
}

void ContentBrowser::Resize(int newWidth, int newHeight)
{
}

void ContentBrowser::Update()
{
    IUserInterfaceElement::Update();
}

void ContentBrowser::RenderModels()
{
    int i = 0;
    for (auto& model : m_assetManager.GetModels())
    {
        auto modelLabel = ICON_FA_CUBES " " + model.first.substr(model.first.rfind("/",-1 ));

        if (ImGui::Selectable(modelLabel.c_str(), m_selectedAsset == model.first))
        {
            m_selectedAsset = model.first;
        }

        if (ImGui::BeginPopupContextItem(modelLabel.c_str()))
        {
            if (ImGui::MenuItem("Add"))
            {
                auto newNode = std::make_shared<ApplicationCore::SceneNode>();
                newNode->SetName(m_selectedAsset.substr(m_selectedAsset.rfind("/",-1 )) + "##" + VulkanUtils::random_string(4));
                for (auto &node: model.second)
                {
                    newNode->AddChild(std::make_shared<ApplicationCore::SceneNode>(*node));
                }
                m_scene.AddNode(newNode);
            }
            if (ImGui::MenuItem("Remove"))
            {

            }
            ImGui::EndPopup();
        }

        i++;
    }

}

void ContentBrowser::RenderMeshes()
{
}
