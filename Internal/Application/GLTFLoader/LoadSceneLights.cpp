//
// Created by wpsimon09 on 02/03/25.
//

#include "LoadSceneLights.hpp"

#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Structs/LightStructs.hpp"
#include "tINI/ini.h"
void ApplicationCore::LoadSceneLights(ApplicationCore::Scene& scene, std::filesystem::path& lightInfoPath)
{
    //=======================
    // POINT LIGHTS
    //=======================
    mINI::INIFile file(lightInfoPath);
    mINI::INIStructure ini ;
    file.read(ini);
    int numOfPointLights = 0;

    if (ini.has("point"))
    {
        numOfPointLights = std::stoi(ini["point"]["count"]);
    }

    for (int i = 0; i < numOfPointLights; i++)
    {
        LightStructs::PointLight pointLightData;
        auto pointLightKey = "point-light-" + std::to_string(i);

        pointLightData.colour = {
            std::stof(ini[pointLightKey]["colr"]),
            std::stof(ini[pointLightKey]["colg"]),
            std::stof(ini[pointLightKey]["colb"]),
            std::stof(ini[pointLightKey]["cola"]) // intensity
        };

        pointLightData.position = {
            std::stof(ini[pointLightKey]["posx"]),
            std::stof(ini[pointLightKey]["posy"]),
            std::stof(ini[pointLightKey]["posz"]),
        };

        pointLightData.constantFactor = std::stof(ini[pointLightKey]["constantfactor"]);
        pointLightData.linearFactor = std::stof(ini[pointLightKey]["linearfactor"]);
        pointLightData.quadraticFactor = std::stof(ini[pointLightKey]["quadraticfactor"]);
        pointLightData.isPointLightInUse = std::stof(ini[pointLightKey]["isinuse"]);

        scene.AddPointLight(&pointLightData);
    }
}
