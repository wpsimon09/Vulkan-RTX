//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_COMPONENT_HPP
#define VULKAN_RTX_COMPONENT_HPP
#include "IconsFontAwesome6.h"
#include <string>

namespace ECS {
struct IComponent
{
    std::string componentLabel = "unknown";
    const char* icon           = ICON_FA_NOTDEF;
    IComponent(const std::string& componentLabel, const char* icon)
        : componentLabel(componentLabel)
        , icon(icon)
    {
    }
};
}  // namespace ECS

#endif  //VULKAN_RTX_COMPONENT_HPP
