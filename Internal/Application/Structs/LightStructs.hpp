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
        glm::vec4 colour {0.2f, 0.2f, 0.4f, 10.0f};

        mutable
        glm::vec3 position;

    };

    struct DirectionalLight
    {
        glm::vec4 colour;

        mutable
        glm::vec3 direction;
    };

    struct SceneLightInfo
    {
        LightStructs::DirectionalLight* DirectionalLightInfo;


        std::vector<LightStructs::PointLight*> PointLightInfos;
        int AddPointLight(PointLight* pointLight)
        {
            PointLightInfos.emplace_back(pointLight);
            CurrentPointLightIndex++;
            return CurrentPointLightIndex-1;
        };

    private:
        int CurrentPointLightIndex = 0;
    };

}

#endif //LIGHTSTRUCTS_HPP
