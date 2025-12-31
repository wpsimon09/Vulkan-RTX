//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_SYSTEM_HPP
#define VULKAN_RTX_SYSTEM_HPP
#include "Application/ECS/Types.hpp"

#include <set>

namespace ECS {

class ISystem
{
  public:
    std::set<ECS::Entity> m_entities;
};
}  // namespace ECS

#endif  //VULKAN_RTX_SYSTEM_HPP
