//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_COMPONENTPANEL_HPP
#define VULKAN_RTX_COMPONENTPANEL_HPP
#include "ComponentDrawUtils.hpp"
#include "Application/ECS/Types.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ECS {
class ECSCoordinator;
}
namespace VEditor {
class ComponentPanel : public VEditor::IUserInterfaceElement
{
  public:
    ComponentPanel(ECS::ECSCoordinator& ecs);
    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

    void SetSelectedEntity(ECS::Entity* entity);

  private:
    ECS::ECSCoordinator&        m_ecs;
    ECS::Entity*                m_selectedEntity = nullptr;
    VEditor::ComponentDrawUtils m_drawCommands;
};

}  // namespace VEditor

#endif  //VULKAN_RTX_COMPONENTPANEL_HPP
