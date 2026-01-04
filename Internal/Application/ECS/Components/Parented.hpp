//
// Created by simon on 04/01/2026.
//

#ifndef VULKAN_RTX_PARENTED_HPP
#define VULKAN_RTX_PARENTED_HPP
#include "Component.hpp"
#include "Application/ECS/Types.hpp"

#include <glm/fwd.hpp>

struct Parented : ECS::IComponent
{
    ECS::Entity* parentEntity        = nullptr;
    glm::mat4    transformFromParent = glm::mat4(1.0f);

    Parented(ECS::Entity* parentEntity, glm::mat4 transformFromParent)
        : IComponent("Parent entity", ICON_FA_PERSON)
        , parentEntity(parentEntity)
        , transformFromParent(transformFromParent)
    {
    }
    Parented()
        : IComponent("Parent entity", ICON_FA_PERSON)
    {
    }
};

#endif  //VULKAN_RTX_PARENTED_HPP
