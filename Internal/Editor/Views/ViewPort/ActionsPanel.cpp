//
// Created by simon on 12/01/2026.
//

#include "ActionsPanel.hpp"

#include "imgui.h"
#include "Application/World/World.hpp"

namespace VEditor {
ActionsPanel::ActionsPanel(ApplicationCore::World& world)
    : m_world(world)
    , m_ecs(world.GetECS())
    , IUserInterfaceElement()
{
}
void ActionsPanel::Render()
{
    bool open = true;
    ImGui::BeginChild("Actions panel", ImVec2(ImGui::GetContentRegionAvail().x * 0.2, 40));
    ImGui::Button("Test");

    ImGui::EndChild();
    IUserInterfaceElement::Render();
}
void ActionsPanel::Resize(int newWidth, int newHeight) {}

void ActionsPanel::Update()
{
    IUserInterfaceElement::Update();
}
void ActionsPanel::RenderAddPopUp() {}
}  // namespace VEditor