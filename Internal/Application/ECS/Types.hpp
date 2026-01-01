//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_TYPES_HPP
#define VULKAN_RTX_TYPES_HPP
#include "vulkan/vulkan.hpp"

#include <bitset>

namespace ECS {

using Entity                           = uint32_t;
constexpr Entity MAX_ENTITIES          = 5000;
using Signature                        = std::bitset<MAX_ENTITIES>;
using ComponentType                    = uint8_t;
constexpr ComponentType MAX_COMPONENTS = 32;


}  // namespace ECS


#endif  //VULKAN_RTX_TYPES_HPP
