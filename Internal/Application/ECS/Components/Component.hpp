//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_COMPONENT_HPP
#define VULKAN_RTX_COMPONENT_HPP
#include <string_view>

namespace ECS {
struct IComponent
{
    std::string name = "unknown";
};
}  // namespace ECS

#endif  //VULKAN_RTX_COMPONENT_HPP
