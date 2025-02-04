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
        mutable
        float intensity; // rgb colour * light strength

        glm::vec3 colour;

        mutable
        glm::vec3 position;

    };

    struct DirectionalLight
    {
        mutable
        float intensity; // rgb colour * light strength

        glm::vec3 colour;


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
