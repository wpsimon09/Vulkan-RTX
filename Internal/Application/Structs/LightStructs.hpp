//
// Created by wpsimon09 on 03/02/25.
//

#ifndef LIGHTSTRUCTS_HPP
#define LIGHTSTRUCTS_HPP
#include <glm/vec3.hpp>

namespace LightStructs
{
    struct PointLight
    {
        glm::vec3 intensity; // rgb colour * light strength
        glm::vec3 position;

    };

    struct DirectionalLight
    {
        mutable
        glm::vec3 intensity; // rgb colour * light strength
        mutable
        glm::vec3 direction;
    };

    struct SceneLightInfo
    {
        DirectionalLight* directionalLight;
        std::array<PointLight*, 100> PointLights;
    };

}

#endif //LIGHTSTRUCTS_HPP
