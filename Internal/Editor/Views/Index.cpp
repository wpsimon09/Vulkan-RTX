//
// Created by wpsimon09 on 21/12/24.
//

#include "Index.hpp"

#include <../../../External/imgui/imgui.h>

VEditor::Index::Index(int width, int height)
    : m_width(width)
    , m_height(height)
{
}

void VEditor::Index::Render()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
    // completely covers the main viewport, giving it a "full-screen" feel.
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Set the parent window's styles to match that of the main viewport:
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);    // No corner rounding on the window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);  // No border around the window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));


    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    bool open = true;

    ImGui::Begin("Index", &open, window_flags);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
    ImGui::PopStyleVar(3);

    ImGui::End();

    IUserInterfaceElement::Render();
}

void VEditor::Index::Resize(int newWidth, int newHeight) {}
