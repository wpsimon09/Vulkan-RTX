//
// Created by wpsimon09 on 04/10/24.
//

#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include <memory>
#include <optional>
#include <vector>
#include "Application/Enums/ClientEnums.hpp"
#include "glm/glm.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"


namespace VulkanCore {
class VDevice;
class VBuffer;
}  // namespace VulkanCore

namespace ApplicationCore {
struct Vertex
{
    glm::vec3 position = {0.0f, 0.0F, 0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 uv = {0.0f, 0.0f};
};

}  // namespace ApplicationCore


#endif  //VERTEXARRAY_HPP
