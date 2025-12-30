//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_TRANSFORMCOMPONENT_HPP
#define VULKAN_RTX_TRANSFORMCOMPONENT_HPP
#include "Component.hpp"

#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>

namespace ECS {

struct TransformComponent : public IComponent
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};
}  // namespace ECS

#endif  //VULKAN_RTX_TRANSFORMCOMPONENT_HPP
