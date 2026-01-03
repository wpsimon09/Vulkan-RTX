//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_COMPONENTPANEL_HPP
#define VULKAN_RTX_COMPONENTPANEL_HPP
#include "ComponentDrawUtils.hpp"
#include "Application/ECS/Types.hpp"
#include "Application/ECS/Components/TransformComponent.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace ECS {
class ECSCoordinator;
}
namespace VEditor {

static const auto V_EDITOR_ADD_COMPONENT_POP_UP_ID = "AddComponentPopUp";

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

  private:
    void CreateAddComponentPopUp();

    template <typename T>
    void AddComponentMenuItem(const char* label)
    {
        if(m_ecs.GetSignatureOf(*m_selectedEntity).test(m_ecs.GetComopnentType<T>()))
        {
            return;
        }
        if(ImGui::MenuItem(label))
        {
            m_ecs.AddComponentTo<T>(*m_selectedEntity, T());
        }
    }
};

}  // namespace VEditor

#endif  //VULKAN_RTX_COMPONENTPANEL_HPP
