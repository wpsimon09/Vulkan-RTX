//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_WORLDOUTLINE_HPP
#define VULKAN_RTX_WORLDOUTLINE_HPP
#include "Application/ECS/Types.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

#include <set>

namespace ApplicationCore {
class World;
}
namespace VEditor {
class ComponentPanel;

class WorldOutline : public VEditor::IUserInterfaceElement
{
  public:
    explicit WorldOutline(ApplicationCore::World& world);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    ECS::Entity             m_selectedEntity;
    std::set<ECS::Entity>   m_selectedEntities;
    ApplicationCore::World& m_world;
    ComponentPanel*         m_componentPanel;
};

}  // namespace VEditor

#endif  //VULKAN_RTX_WORLDOUTLINE_HPP
