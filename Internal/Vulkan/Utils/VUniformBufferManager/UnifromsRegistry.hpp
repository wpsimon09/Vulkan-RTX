//
// Created by wpsimon09 on 31/10/24.
//

#ifndef UNIFROMDEFINITION_HPP
#define UNIFROMDEFINITION_HPP
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"

namespace VulkanCore
{
    class VBuffer;
}

namespace PerFrameUBO
{
    struct GlobalUniform
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec4 lightPosition = glm::vec4(2.0f, 10.0f, 2.0f,1.0f);
        glm::vec4 playerPosition;
        glm::vec4 cameraInformation;
    };
}

namespace PerObjectUBO
{
    struct ObjectDataUniform
    {
        glm::mat4 model;
        glm::mat4 normalMatrix;
    };
}


#endif //UNIFROMDEFINITION_HPP
