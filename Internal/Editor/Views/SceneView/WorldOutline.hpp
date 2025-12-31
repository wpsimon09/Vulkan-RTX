//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_WORLDOUTLINE_HPP
#define VULKAN_RTX_WORLDOUTLINE_HPP
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ApplicationCore {
class World;
}
namespace VEditor {

class WorldOutline : public VEditor::IUserInterfaceElement
{
  public:
    explicit WorldOutline(ApplicationCore::World& world);

    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;

  private:
    ApplicationCore::World& m_world;
};

}  // namespace VEditor

#endif  //VULKAN_RTX_WORLDOUTLINE_HPP
