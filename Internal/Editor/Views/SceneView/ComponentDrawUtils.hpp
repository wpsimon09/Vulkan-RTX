//
// Created by simon on 01/01/2026.
//

#ifndef VULKAN_RTX_COMPONENTDRAWUTILS_HPP
#define VULKAN_RTX_COMPONENTDRAWUTILS_HPP
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
};

}  // namespace VEditor

#endif  //VULKAN_RTX_COMPONENTDRAWUTILS_HPP
