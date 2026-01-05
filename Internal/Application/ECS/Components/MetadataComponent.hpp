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
    char        entityName[60] = "unknown entity";
    uint64_t    uuid{};  // TODO
    std::string tag  = {};
    const char* icon = ICON_FA_CUBES_STACKED;
    MetadataComponent()
        : IComponent("Meta data component", ICON_FA_INFO)

    {
    }

    MetadataComponent(const char* entityName, const char* icon, uint64_t uuid, std::string tag)
        : IComponent("Meta data component", ICON_FA_INFO)
        , uuid(uuid)
        , tag(tag)
        , icon(icon)
    {
        std::strcpy(this->entityName, entityName);
    }

    friend bool operator==(const MetadataComponent& lhs, const MetadataComponent& rhs)
    {
        return std::strcmp(lhs.entityName, rhs.entityName) == 0 && lhs.uuid == rhs.uuid && lhs.tag == rhs.tag;
    }

    friend bool operator!=(const MetadataComponent& lhs, const MetadataComponent& rhs) { return !(lhs == rhs); }
};
}  // namespace ECS
#endif  //VULKAN_RTX_METADATACOMPONENT_HPP
