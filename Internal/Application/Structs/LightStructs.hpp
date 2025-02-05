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
        LightStructs::DirectionalLight* DirectionalLightInfo;



        std::array<LightStructs::PointLight*, 100> PointLightInfos;
        int AddPointLight(PointLight* pointLight)
        {
            PointLightInfos[CurrentPointLightIndex] = pointLight;
            CurrentPointLightIndex++;
            return CurrentPointLightIndex-1;
        };

    private:
        int CurrentPointLightIndex = 0;
    };

}

#endif //LIGHTSTRUCTS_HPP
