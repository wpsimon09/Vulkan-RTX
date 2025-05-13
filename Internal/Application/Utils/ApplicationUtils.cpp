#include "ApplicationUtils.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include <glm/ext/vector_float3.hpp>
#include "Application/VertexArray/VertexArray.hpp"

namespace ApplicationCore {
VulkanStructs::VBounds CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices)
{
    //========================
    // CALCULATE BOUNDING BOX
    //========================
    glm::vec3 maxPos = vertices[0].position;
    glm::vec3 minPos = vertices[0].position;

    for(const auto& i : vertices)
    {
        minPos = glm::min(minPos, i.position);
        maxPos = glm::max(maxPos, i.position);
    }
    VulkanStructs::VBounds bounds = {};
    bounds.origin                = (maxPos + minPos) / 2.f;
    bounds.extents               = (maxPos - minPos) / 2.f;
    bounds.max                   = maxPos;
    bounds.min                   = minPos;
    bounds.radius                = glm::length(bounds.extents);

    return bounds;
}
}  // namespace ApplicationCore

std::string ThemeToString(ETheme theme)
{
    switch(theme)
    {
        case ETheme::Light:
            return "Light";
        case ETheme::Dark:
            return "Dark";
        default:
            return "Dark";
    }
}
