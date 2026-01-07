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
    ComponentPanel(ECS::ECSCoordinator& ecs, std::vector<ECS::Entity>& selectedEntities);
    void Render() override;
    void Resize(int newWidth, int newHeight) override;
    void Update() override;
    void SetSharedSignature(ECS::Signature signature);

  private:
    ECS::ECSCoordinator&        m_ecs;
    VEditor::ComponentDrawUtils m_drawCommands;

    std::vector<ECS::Entity>& m_selectedEntities;
    ECS::Signature            m_sharedSignature;

  private:
    void CreateAddComponentPopUp();

    template <typename T>
    void AddComponentMenuItem(const char* label)
    {
        if(ImGui::MenuItem(label))
        {
            for(auto& entity : m_selectedEntities)
            {
                m_ecs.AddComponentTo<T>(entity, T());
            }
        }
    }
};

}  // namespace VEditor

#endif  //VULKAN_RTX_COMPONENTPANEL_HPP
