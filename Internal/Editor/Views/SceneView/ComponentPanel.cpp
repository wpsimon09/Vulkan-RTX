//
// Created by simon on 31/12/2025.
//

#include "ComponentPanel.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/AtmosphereComponent.hpp"
#include "Application/ECS/Components/FogComponent.hpp"
#include "Application/ECS/Components/LightComponents.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/ECS/Components/StaticMeshComponent.hpp"
#include "Application/ECS/Components/TransformComponent.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {

ComponentPanel::ComponentPanel(ECS::ECSCoordinator& ecs, std::vector<ECS::Entity>& selectedEntities)
    : m_ecs(ecs)
    , m_drawCommands(ecs)
    , m_selectedEntities(selectedEntities)
{
}

void ComponentPanel::Render()
{
    ImGui::Begin(ICON_FA_CUBES "Components");

    if(!m_selectedEntities.empty())
    {
        m_drawCommands.DrawMultiSelect(m_sharedSignature, m_selectedEntities);

        ImVec2 cursorPos = ImGui::GetCursorPos();
        cursorPos.y += 10;
        ImGui::SetCursorPos(cursorPos);
        if(ImGui::Button(ICON_FA_CIRCLE_PLUS " Add component"))
        {
            ImGui::OpenPopup(V_EDITOR_ADD_COMPONENT_POP_UP_ID);
        }
        CreateAddComponentPopUp();
    }
    else
    {
        ImGui::Text(ICON_FA_CIRCLE_EXCLAMATION " Nothing selected");
    }

    // clear the list of entities to be editted
    m_selectedEntities.clear();

    ImGui::End();
    IUserInterfaceElement::Render();
}

void ComponentPanel::Resize(int newWidth, int newHeight) {}

void ComponentPanel::Update()
{
    IUserInterfaceElement::Update();
}

void ComponentPanel::SetSharedSignature(ECS::Signature signature)
{
    m_sharedSignature = signature;
}

void ComponentPanel::CreateAddComponentPopUp()
{
    float gap = 10.0f;
    if(ImGui::BeginPopupContextItem(V_EDITOR_ADD_COMPONENT_POP_UP_ID))
    {
        if(ImGui::TreeNode(ICON_FA_TOOLBOX " Common"))
        {
            AddComponentMenuItem<ECS::TransformComponent>(ICON_FA_MAP " Transformation");
            AddComponentMenuItem<ECS::MetadataComponent>(ICON_FA_CIRCLE_INFO " Metadata");
            AddComponentMenuItem<ECS::StaticMeshComponent>(ICON_FA_CUBE " Static mesh component");
            ImGui::Dummy(ImVec2(0.0, gap));

            ImGui::TreePop();
        }
        if(ImGui::TreeNode(ICON_FA_LIGHTBULB " Lightning"))
        {
            AddComponentMenuItem<ECS::DirectionalLightComponent>(ICON_FA_SUN " Directional light component");
            AddComponentMenuItem<ECS::AreaLightComponent>(ICON_FA_RECTANGLE_XMARK " Area light component");
            AddComponentMenuItem<ECS::PointLightComponent>(ICON_FA_LIGHTBULB " Point light component");

            AddComponentMenuItem<ECS::EnvironemntLightComponent>(ICON_FA_MOUNTAIN_CITY " Environment map component");

            ImGui::Dummy(ImVec2(0.0, gap));
            ImGui::TreePop();
        }
        if(ImGui::TreeNode(ICON_FA_EYE " VFX"))
        {
            AddComponentMenuItem<ECS::FogComponent>(ICON_FA_SMOG " Fog component");
            AddComponentMenuItem<ECS::AtmosphereComponent>(ICON_FA_MOUNTAIN_SUN " Atmosphere component");
            ImGui::Dummy(ImVec2(0.0, gap));

            ImGui::TreePop();
        }

        ImGui::EndPopup();
    }
}

}  // namespace VEditor