//
// Created by simon on 31/12/2025.
//

#include "WorldOutline.hpp"

#include "ComponentPanel.hpp"
#include "IconsFontAwesome6.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/World/World.hpp"
#include "imgui.h"
#include "Application/Utils/ApplicationUtils.hpp"

#include <string_view>

namespace VEditor {
WorldOutline::WorldOutline(WindowManager& windowManager, ApplicationCore::World& world)
    : m_world(world)
    , m_windowManager(windowManager)
{
    auto ecs = &m_world.GetECS();

    m_uiChildren.push_back(std::make_unique<VEditor::ComponentPanel>(m_world.GetECS(), m_selectedEntities));
    m_componentPanel = dynamic_cast<ComponentPanel*>(m_uiChildren.back().get());
}
void WorldOutline::Render()
{
    auto ecs = &m_world.GetECS();

    ImGui::Begin(ICON_FA_MOUNTAIN " World outline");

    ImGui::InputText(ICON_FA_MAGNIFYING_GLASS "Search", m_searchPhrase, IM_ARRAYSIZE(m_searchPhrase));

    if(ImGui::BeginChild("##World outline", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
    {
        std::vector<ECS::Entity> visibleEntities;
        visibleEntities.clear();

        for(ECS::Entity entity = 0; entity < ecs->GetAllAliveEntities(); entity++)
        {
            auto&       meta  = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
            std::string label = std::string(meta.icon) + " " + meta.entityName;

            if(StringContains(label, m_searchPhrase))
            {
                visibleEntities.push_back(entity);
            }
        }

        ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
        ImGuiMultiSelectIO*   ms_io = ImGui::BeginMultiSelect(flags, m_selection.Size, ecs->GetAllAliveEntities());
        m_selection.ApplyRequests(ms_io);

        const bool wantDelete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && m_selection.Size > 0) || m_requestDelete;

        const int focus_idx = wantDelete ? m_selection.ApplyDeletionPreLoop(ms_io, ecs->GetAllAliveEntities()) : -1;

        m_requestDelete = false;

        ImGuiListClipper clipper;
        clipper.Begin(visibleEntities.size());
        if(focus_idx != -1)
            clipper.IncludeItemByIndex(focus_idx);

        if(ms_io->RangeSrcItem != -1)
            clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem);


        while(clipper.Step())
        {
            for(ECS::Entity entity = clipper.DisplayStart; entity < clipper.DisplayEnd; entity++)
            {
                auto&       data  = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
                std::string label = std::string(data.icon) + " " + data.entityName + "##" + std::to_string(entity);
                std::string searchPrase = m_searchPhrase;

                ImGui::PushID(entity);
                bool item_is_selected = m_selection.Contains((ImGuiID)entity);
                ImGui::SetNextItemSelectionUserData(entity);
                ImGui::Selectable(label.c_str(), item_is_selected);

                if(focus_idx == entity)
                {
                    ImGui::SetKeyboardFocusHere(-1);
                }

                if(ImGui::BeginPopupContextItem())
                {
                    ImGui::BeginDisabled(m_selection.Size == 0);
                    if(ImGui::Selectable(ICON_FA_TRASH_CAN " Delete"))
                    {
                        m_requestDelete = true;
                    }
                    ImGui::EndDisabled();

                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
        }

        ms_io = ImGui::EndMultiSelect();
        m_selection.ApplyRequests(ms_io);

        if(m_selection.Size > 0)
        {
            m_componentPanel->SetSharedSignature(ParseSelectionAndProcessDelete(wantDelete));
        }
    }


    ImGui::EndChild();
    ImGui::End();

    // converts ImGui storage of selected entities to the std::vector<Entitiy> and returns shared signature

    IUserInterfaceElement::Render();
}

void WorldOutline::Resize(int newWidth, int newHeight) {}

void WorldOutline::Update()
{
    IUserInterfaceElement::Update();
}

ECS::Signature WorldOutline::ParseSelectionAndProcessDelete(bool wantDelete)
{
    if(wantDelete)
    {
        void*   it = nullptr;
        ImGuiID id;

        while(m_selection.GetNextSelectedItem(&it, &id))
        {
            m_world.GetECS().DestroyEntity((ECS::Entity)id);
        }
        m_selectedEntities.clear();
        return 0;
    }
    auto ecs = &m_world.GetECS();

    ImGuiID        id = 0;
    void*          it = NULL;
    int            i  = 0;
    ECS::Signature sharedSignature;

    // parse entities to the vector
    m_selectedEntities.reserve(m_selection.Size);

    while(m_selection.GetNextSelectedItem(&it, &id))
    {
        auto entity    = (ECS::Entity)id;
        auto signature = ecs->GetSignatureOf(entity);

        m_selectedEntities.push_back(entity);

        if(i == 0)
        {
            // get the shared signatuer to know which components to draw
            sharedSignature = signature;
        }

        sharedSignature &= signature;
        i++;
    }

    return sharedSignature;
}


}  // namespace VEditor