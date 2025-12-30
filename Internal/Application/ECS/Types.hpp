//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_ENTITY_HPP
#define VULKAN_RTX_ENTITY_HPP
#include "vulkan/vulkan.hpp"

#include <bitset>

namespace ECS {

using Entity                           = uint32_t;
constexpr Entity MAX_ENTITIES          = 10000;
using Signature                        = std::bitset<MAX_ENTITIES>;
using ComponentType                    = uint8_t;
constexpr ComponentType MAX_COMPONENTS = 32;

}  // namespace ECS


#endif  //VULKAN_RTX_ENTITY_HPP
