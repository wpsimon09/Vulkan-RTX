//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_WORLDOUTLINE_HPP
#define VULKAN_RTX_WORLDOUTLINE_HPP
#include "ImGuiSelectionWithDeletion.hpp"
#include "imgui.h"
#include "Application/ECS/Types.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

#include <set>


class WindowManager;
namespace ApplicationCore {
class World;
}
namespace VEditor {
class ComponentPanel;

static const auto WORLD_OUTLINE_OPTIONS_POP_UP = "WORLD_OUTLINE_OPTIONS_POP_UP";


class WorldOutline : public VEditor::IUserInterfaceElement
{
  public:
    explicit WorldOutline(WindowManager& windowManager, ApplicationCore::World& world);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    void           RenderWorldOutlineOptions();
    ECS::Signature ParseSelectionAndPreformActions(bool wantDelete, bool wantCopy, bool wantPaste);

  private:
    ECS::Entity              m_selectedEntity;
    ApplicationCore::World&  m_world;
    ComponentPanel*          m_componentPanel;
    SelectionContainer       m_selection;
    WindowManager&           m_windowManager;
    std::vector<ECS::Entity> m_selectedEntities{};
    bool                     m_requestDelete;
    bool                     m_requestCopy;
    bool                     m_requestPaste;
    char                     m_searchPhrase[70] = "";
};

}  // namespace VEditor

#endif  //VULKAN_RTX_WORLDOUTLINE_HPP
