//
// Created by simon on 01/01/2026.
//

#ifndef VULKAN_RTX_COMPONENTDRAWUTILS_HPP
#define VULKAN_RTX_COMPONENTDRAWUTILS_HPP

#include "IconsFontAwesome6.h"
#include "imgui_internal.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Types.hpp"


#include <functional>
#include <unordered_map>

namespace ECS {
class ECSCoordinator;
}
namespace VEditor {

class ComponentDrawUtils
{
  public:
    explicit ComponentDrawUtils(ECS::ECSCoordinator& ecs);


    void DrawMultiSelect(ImGuiSelectionBasicStorage* storage);
    void Draw(ECS::Entity entity);

  private:
    void Draw(ECS::Signature signature, const std::vector<ECS::Entity>& entities);

    void DrawTransform(ECS::Entity entity);
    void DrawMetadataComponent(ECS::Entity entity);
    void DrawStaticMeshComponent(ECS::Entity entity);

    void DrawTransformMultiselect(const std::vector<ECS::Entity>& entities);
    void DrawMetadataComponent(const std::vector<ECS::Entity>& entities);
    void DrawStaticMeshComponent(const std::vector<ECS::Entity>& entities);

  private:
    ECS::ECSCoordinator&                                                     m_ecs;
    std::unordered_map<ECS::ComponentType, std::function<void(ECS::Entity)>> m_drawFunctions;

    std::unordered_map<ECS::ComponentType, std::function<void(const std::vector<ECS::Entity>&)>> m_drawMultiSelectFunctions;
    //=================================
    // State variables
    float                    m_uniformScaleScalar = 1.0;
    bool                     m_isUniformScaleOn   = false;
    std::vector<ECS::Entity> m_entitiesToEdit     = {};


  private:
    template <typename T>
    void RenderOptions(ECS::Entity entity)
    {
        if(ImGui::BeginPopupContextItem())
        {
            if(ImGui::MenuItem(ICON_FA_TRASH_CAN " Remove from entity"))
            {
                m_ecs.RemoveComponentFrom<T>(entity);
            }
            ImGui::EndPopup();
        }
    }

    template <typename T>
    void ApplyToAll(T changedComponent, const std::vector<ECS::Entity>& entities)
    {
        for(auto& e : entities)
        {
            m_ecs.SetComponentValue<T>(changedComponent, e);
        }
    }
};

}  // namespace VEditor

#endif  //VULKAN_RTX_COMPONENTDRAWUTILS_HPP
