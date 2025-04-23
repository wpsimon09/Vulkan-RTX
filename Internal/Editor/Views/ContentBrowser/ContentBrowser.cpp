//
// Created by wpsimon09 on 03/03/25.
//

#include "ContentBrowser.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/SkyBoxMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"

ContentBrowser::ContentBrowser(ApplicationCore::AssetsManager& assetManager, ApplicationCore::Scene& scene)
    : IUserInterfaceElement()
    , m_assetManager(assetManager)
    , m_scene(scene)
{
}

void ContentBrowser::Render()
{
    ImGui::Begin(ICON_FA_FOLDER_OPEN " ContentBrowser");

    ImGui::BeginTabBar("Items");
    {
        if(ImGui::BeginTabItem("Models"))
        {
            RenderModels();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Meshes"))
        {
            RenderMeshes();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Effects"))
        {
            RenderEffects();
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("HDRs"))
        {
            RenderHDRs();
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();


    ImGui::End();
    IUserInterfaceElement::Render();
}

void ContentBrowser::Resize(int newWidth, int newHeight) {}

void ContentBrowser::Update()
{
    IUserInterfaceElement::Update();
}

void ContentBrowser::RenderModels()
{
    int i = 0;
    for(auto& model : m_assetManager.GetModels())
    {
        auto modelLabel = ICON_FA_CUBES " " + model.first.substr(model.first.rfind("/"));

        if(ImGui::Selectable(modelLabel.c_str(), m_selectedAsset == model.first))
        {
            m_selectedAsset = model.first;
        }

        if(ImGui::BeginPopupContextItem(modelLabel.c_str()))
        {
            if(ImGui::MenuItem("Add"))
            {
                auto newNode = std::make_shared<ApplicationCore::SceneNode>();
                newNode->SetName(m_selectedAsset.substr(m_selectedAsset.rfind('/') + 1) + "##" + VulkanUtils::random_string(4));
                for(auto& node : model.second)
                {
                    auto subNode = std::make_shared<ApplicationCore::SceneNode>(*node);
                    newNode->AddChild(std::move(subNode));
                }
                m_scene.AddNode(std::move(newNode));
            }
            if(ImGui::MenuItem("Remove"))
            {
            }
            ImGui::EndPopup();
        }

        i++;
    }
}

void ContentBrowser::RenderMeshes()
{
    int i = 0;
    for(auto& mesh : m_assetManager.GetMeshes())
    {
        auto meshLabel = ICON_FA_CUBE " " + mesh.first;

        if(ImGui::Selectable(meshLabel.c_str(), m_selectedAsset == mesh.first))
        {
            m_selectedAsset = mesh.first;
        }

        if(ImGui::BeginPopupContextItem(meshLabel.c_str()))
        {
            if(ImGui::MenuItem("Add"))
            {
                auto newMesh =
                    std::make_shared<ApplicationCore::StaticMesh>(*m_assetManager.GetMeshes()[m_selectedAsset].get());
                auto newNode = std::make_shared<ApplicationCore::SceneNode>(newMesh);
                newNode->SetName(meshLabel + VulkanUtils::random_string(4));
                m_scene.AddNode(std::move(newNode));
            }
            if(ImGui::MenuItem("Remove"))
            {
            }
            ImGui::EndPopup();
        }

        i++;
    }
}

void ContentBrowser::RenderEffects()
{
    int i = 0;
    for(auto& effect : m_assetManager.GetEffects())
    {
        auto& currentEffect = effect.second;
        auto  meshLabel     = ICON_FA_WAND_MAGIC " " + currentEffect->GetName();

        if(ImGui::Selectable(meshLabel.c_str(), m_selectedAsset == currentEffect->GetName()))
        {
            m_selectedAsset = currentEffect->GetName();
        }

        if(ImGui::BeginPopupContextItem(meshLabel.c_str()))
        {
            if(ImGui::MenuItem("Change"))
            {
                Utils::Logger::LogInfoClient("Changin effect");
            }
            if(ImGui::MenuItem("Remove"))
            {
            }
            ImGui::EndPopup();
        }

        i++;
    }
}

void ContentBrowser::RenderHDRs()
{
    int i = 0;
    for(auto& hdr : m_assetManager.GetAllSkyBoxMaterials())
    {
        auto& currentHDR = hdr;
        auto  hdrLabel   = ICON_FA_MOUNTAIN_SUN " " + currentHDR->GetMaterialName();

        if(ImGui::Selectable(hdrLabel.c_str(), m_selectedAsset == currentHDR->GetMaterialName()))
        {
            m_selectedAsset = currentHDR->GetMaterialName();
        }

        if(ImGui::BeginPopupContextItem(hdrLabel.c_str()))
        {
            if(ImGui::MenuItem("Change"))
            {
                //Utils::Logger::LogInfoClient("Changin effect");
            }
            if(ImGui::MenuItem("Remove"))
            {
                m_assetManager.DestroySkyBoxMaterial(hdr->GetMaterialName());
            }
            ImGui::EndPopup();
        }

        i++;
    }
}
