//
// Created by wpsimon09 on 31/10/24.
//

#ifndef UNIFROMDEFINITION_HPP
#define UNIFROMDEFINITION_HPP
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace VulkanCore
{
    class VBuffer;
}

namespace PerFrameUBO
{
    struct CameraUniform
    {
        glm::mat4 view;
        glm::mat4 proj;
        std::vector<std::unique_ptr<VulkanCore::VBuffer>> buffer;
    };
}

namespace PerMaterialUBO
{

}

namespace PerObjectUBO
{

}


#endif //UNIFROMDEFINITION_HPP
