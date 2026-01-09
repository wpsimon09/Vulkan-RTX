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

    //====================================
    // Filter entities based on the search
    // - does entity mach the search ?
    // - store its index to the visible entieis
    // - continue to loop through it as ususla
    std::vector<ECS::Entity> visibleEntities;
    visibleEntities.clear();
    if(ImGui::BeginChild("##World outline", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
    {

        for(auto& entity : ecs->GetAliveEntities())
        {
            auto&       meta  = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
            std::string label = std::string(meta.icon) + " " + meta.entityName;

            if(StringContains(label, m_searchPhrase))
            {
                visibleEntities.push_back(entity);
            }
        }

        //=========================================
        // Create multi select context and flags
        ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
        ImGuiMultiSelectIO*   ms_io = ImGui::BeginMultiSelect(flags, m_selection.Size, ecs->GetAllAliveEntities());
        m_selection.ApplyRequests(ms_io);

        //==================================
        // Either shortcut key was pressed
        // or last frame requested this action through context menu
        const bool wantDelete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && m_selection.Size > 0) || m_requestDelete;
        const bool wantCopy  = (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C) && m_selection.Size > 0) || m_requestCopy;
        const bool wantPaste = ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_P) || m_requestPaste;

        const int focus_idx = wantDelete ? m_selection.ApplyDeletionPreLoop(ms_io, ecs->GetAllAliveEntities()) : -1;

        // reset requests from previous frame
        m_requestDelete = false;
        m_requestCopy   = false;
        m_requestPaste  = false;

        //======================================
        // Configure clipper for performance
        ImGuiListClipper clipper;
        clipper.Begin(visibleEntities.size());
        if(focus_idx != -1)
            clipper.IncludeItemByIndex(focus_idx);

        if(ms_io->RangeSrcItem != -1)
            clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem);


        //=================================================================
        // Go through the list of all entities that passed the filter check
        while(clipper.Step())
        {
            for(ECS::Entity n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
            {
                //=================================================
                // Get data of entity (name, icon etc...)
                auto&       entity = visibleEntities[n];
                auto&       data   = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
                std::string label  = std::string(data.icon) + " " + data.entityName + "##" + std::to_string(entity);
                std::string searchPrase = m_searchPhrase;

                ImGui::PushID(entity);
                bool item_is_selected = m_selection.Contains((ImGuiID)entity);

                // here I have to index to the array of visible entities to get the correct one
                ImGui::SetNextItemSelectionUserData(entity);
                ImGui::Selectable(label.c_str(), item_is_selected);

                if(focus_idx == entity)
                {
                    ImGui::SetKeyboardFocusHere(-1);
                }

                //==========================================
                // Right mouse button pop up
                // - this sets what should happen next frame
                if(ImGui::BeginPopupContextItem())
                {
                    ImGui::BeginDisabled(m_selection.Size == 0);
                    if(ImGui::Selectable(ICON_FA_TRASH_CAN " Delete"))
                    {
                        m_requestDelete = true;
                    }
                    if(ImGui::Selectable(ICON_FA_COPY " Copy"))
                    {
                        m_requestCopy = true;
                    }

                    if(ImGui::Selectable(ICON_FA_PASTE " Paste"))
                    {
                        m_requestPaste = true;
                    }
                    ImGui::EndDisabled();

                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
        }

        ms_io = ImGui::EndMultiSelect();
        m_selection.ApplyRequests(ms_io);

        //=========================================================
        // Retrieve all selected entities and process their delete
        if(m_selection.Size > 0)
        {
            m_componentPanel->SetSharedSignature(ParseSelectionAndPreformActions(wantDelete, wantCopy, wantPaste));
        }
    }

    ImGui::EndChild();
    ImGui::End();
    IUserInterfaceElement::Render();
}

void WorldOutline::Resize(int newWidth, int newHeight) {}

void WorldOutline::Update()
{
    IUserInterfaceElement::Update();
}

ECS::Signature WorldOutline::ParseSelectionAndPreformActions(bool wantDelete, bool wantCopy, bool wantPaste)
{
    auto ecs = &m_world.GetECS();

    if(wantDelete)
    {
        void*   it = nullptr;
        ImGuiID id;

        while(m_selection.GetNextSelectedItem(&it, &id))
        {
            ecs->DestroyEntity((ECS::Entity)id);
        }
        m_selectedEntities.clear();
        return 0;
    }
    if(wantCopy)
    {
        Utils::Logger::LogInfo("copying");
    }
    if(wantPaste)
    {
        Utils::Logger::LogInfo("pasting");
    }


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