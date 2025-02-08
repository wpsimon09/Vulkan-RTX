//
// Created by wpsimon09 on 03/02/25.
//

#ifndef LIGHTSTRUCTS_HPP
#define LIGHTSTRUCTS_HPP
#include <glm/vec3.hpp>

namespace LightStructs
{
    struct AreaLight
    {

    };

    struct PointLight
    {
        mutable
        glm::vec4 colour;

        mutable
        glm::vec3 position;

        float constantFactor = 1.0f;

        float linearFactor = 0.045f;

        float quadraticFactor = 0.0075f;

        bool useAdvancedAttentuation = 0;
        bool isPointLightInUse = false;

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
            pointLight->isPointLightInUse = true;
            CurrentPointLightIndex++;
            return CurrentPointLightIndex-1;
        };

    private:
        int CurrentPointLightIndex = 0;
    };

}

#endif //LIGHTSTRUCTS_HPP
