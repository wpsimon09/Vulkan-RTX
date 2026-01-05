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
#include <string_view>

namespace VEditor {
WorldOutline::WorldOutline(ApplicationCore::World& world)
    : m_world(world)
{
    auto ecs = &m_world.GetECS();

    m_uiChildren.push_back(std::make_unique<VEditor::ComponentPanel>(m_world.GetECS(), &m_selection));
    m_componentPanel = dynamic_cast<ComponentPanel*>(m_uiChildren.back().get());
}
void WorldOutline::Render()
{
    auto ecs = &m_world.GetECS();

    ImGui::Begin(ICON_FA_MOUNTAIN " World outline");

    ImGui::InputText(ICON_FA_MAGNIFYING_GLASS "Search", m_searchPhrase, IM_ARRAYSIZE(m_searchPhrase));

    if(ImGui::BeginChild("##World outline", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
    {
        ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
        ImGuiMultiSelectIO*   ms_io = ImGui::BeginMultiSelect(flags, m_selection.Size, ecs->GetAllAliveEntities());
        m_selection.ApplyRequests(ms_io);

        for(ECS::Entity entity = 0; entity < ecs->GetAllAliveEntities(); entity++)
        {
            auto&       data  = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
            std::string label = std::string(data.icon) + " " + data.entityName + "##" + std::to_string(entity);
            if(StringContains(label, std::string_view(m_searchPhrase))
                bool item_is_selected = m_selection.Contains((ImGuiID)entity);
            ImGui::SetNextItemSelectionUserData(entity);
            ImGui::Selectable(label.c_str(), item_is_selected);
        }

        ms_io = ImGui::EndMultiSelect();
        m_selection.ApplyRequests(ms_io);
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


}  // namespace VEditor