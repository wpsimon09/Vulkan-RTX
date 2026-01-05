//
// Created by simon on 31/12/2025.
//

#include "ComponentPanel.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/ECS/Components/StaticMeshComponent.hpp"
#include "Application/ECS/Components/TransformComponent.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {

ComponentPanel::ComponentPanel(ECS::ECSCoordinator& ecs, ImGuiSelectionBasicStorage* multiselectStorage)
    : m_ecs(ecs)
    , m_drawCommands(ecs)
    , m_selectionStorage(multiselectStorage)
{
}

void ComponentPanel::Render()
{
    ImGui::Begin(ICON_FA_CUBES "Components");

    ImGuiID        id = 0;
    void*          it = NULL;
    int            i  = 0;
    ECS::Signature sharedSignature;

    // parse entities to the vector
    m_selectedEntities.reserve(m_selectionStorage->Size);

    while(m_selectionStorage->GetNextSelectedItem(&it, &id))
    {
        auto entity    = (ECS::Entity)id;
        auto signature = m_ecs.GetSignatureOf(entity);

        m_selectedEntities.push_back(entity);

        if(i == 0)
        {
            // get the shared signatuer to know which components to draw
            sharedSignature = signature;
        }

        sharedSignature &= signature;
        i++;
    }

    if(m_selectionStorage->Size > 0)
    {
        m_drawCommands.DrawMultiSelect(sharedSignature, m_selectedEntities);

        ImVec2 cursorPos = ImGui::GetCursorPos();
        cursorPos.y += 10;
        ImGui::SetCursorPos(cursorPos);
        if(ImGui::Button(ICON_FA_CIRCLE_PLUS " Add component"))
        {
            ImGui::OpenPopup(V_EDITOR_ADD_COMPONENT_POP_UP_ID);
        }


        CreateAddComponentPopUp();
    }
    /*  if(m_selectedEntity != nullptr)
    {
        m_drawCommands.Draw(*m_selectedEntity);



    } */
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
void ComponentPanel::SetSelectedEntity(ECS::Entity* entity)
{
    m_selectedEntity = entity;
}
void ComponentPanel::CreateAddComponentPopUp()
{
    if(ImGui::BeginPopupContextItem(V_EDITOR_ADD_COMPONENT_POP_UP_ID))
    {
        AddComponentMenuItem<ECS::TransformComponent>(ICON_FA_MAP " Transformation");
        AddComponentMenuItem<ECS::MetadataComponent>(ICON_FA_CIRCLE_INFO "Metadata");
        AddComponentMenuItem<ECS::StaticMeshComponent>(ICON_FA_CUBE "Static mesh component");
        ImGui::EndPopup();
    }
}

}  // namespace VEditor