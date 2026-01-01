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

    void Draw(ECS::Entity entity);

  private:
    void DrawTransform(ECS::Entity entity);

    void DrawMetadataComponent(ECS::Entity entity);

  private:
    ECS::ECSCoordinator&                                                     m_ecs;
    std::unordered_map<ECS::ComponentType, std::function<void(ECS::Entity)>> m_drawFunctions;
    //=================================
    // State variables
    float m_uniformScaleScalar = 1.0;
    bool  m_isUniformScaleOn   = false;

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
};

}  // namespace VEditor

#endif  //VULKAN_RTX_COMPONENTDRAWUTILS_HPP
