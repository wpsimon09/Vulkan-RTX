//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_METADATACOMPONENT_HPP
#define VULKAN_RTX_METADATACOMPONENT_HPP
#include "Component.hpp"
#include "IconsFontAwesome6.h"

#include <cstdint>
#include <string>

namespace ECS {

struct MetadataComponent : public ECS::IComponent
{
    std::string entityName = "unknown entity";
    uint64_t    uuid{};  // TODO
    std::string tag  = {};
    const char* icon = ICON_FA_CUBES_STACKED;
    MetadataComponent()
        : uuid(0)
    {
        name = "Meta data component";
    }
    MetadataComponent(const std::string& name, const char* icon, uint64_t uuid, std::string tag)
        : entityName(name)
        , uuid(uuid)
        , tag(tag)
        , icon(icon)
    {
    }
};

}  // namespace ECS
#endif  //VULKAN_RTX_METADATACOMPONENT_HPP
