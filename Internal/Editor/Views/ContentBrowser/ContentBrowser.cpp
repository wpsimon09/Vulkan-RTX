//
// Created by wpsimon09 on 03/03/25.
//

#include "ContentBrowser.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Scene/Scene.hpp"

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
            if (!m_selectedAsset.empty())
            {
                if (ImGui::BeginPopupContextWindow("Actions"))
                {
                  if (ImGui::MenuItem("Move to scene"))
                  {
                      for (auto &n:m_assetManager.GetModel(m_selectedAsset))
                      {
                          m_scene.AddNode(n);
                      }
                  }
                    ImGui::EndPopup();
                }

            }
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
        if (i % 5 != 0) { ImGui::SameLine(); }

        if (ImGui::Selectable(modelLabel.c_str(), m_selectedAsset == model.first))
        {
            m_selectedAsset = model.first;
        }

        if (ImGui::BeginPopupContextItem(modelLabel.c_str()))
        {
            if (ImGui::MenuItem("Add"))
            {
                for (auto& node)
            }
            if (ImGui::MenuItem("Remove"))
            {
                // TODO: Implement remove functionality
            }
            ImGui::EndPopup();
        }

        i++;
    }

}

void ContentBrowser::RenderMeshes()
{
}
