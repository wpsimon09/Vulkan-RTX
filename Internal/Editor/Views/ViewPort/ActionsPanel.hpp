//
// Created by simon on 12/01/2026.
//

#ifndef VULKAN_RTX_ACTIONSPANEL_HPP
#define VULKAN_RTX_ACTIONSPANEL_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ECS {
class ECSCoordinator;
}
namespace ApplicationCore {
class World;
}
namespace VEditor {

constexpr auto ADD_POP_UP_ID = "ADD_POP_UP";

class ActionsPanel : public IUserInterfaceElement
{
  public:
    ActionsPanel(ApplicationCore::World& world);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    void                    RenderAddPopUp();
    ECS::ECSCoordinator&    m_ecs;
    ApplicationCore::World& m_world;

  private:
    void RenderAddNewPopUp();
};
};  // namespace VEditor

#endif  //VULKAN_RTX_ACTIONSPANEL_HPP
