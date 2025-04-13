//
// Created by wpsimon09 on 02/03/25.
//

#include "LoadSceneLights.hpp"

#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "tINI/ini.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

void ApplicationCore::LoadSceneLights(ApplicationCore::Scene& scene, std::filesystem::path& lightInfoPath)
{
  mINI::INIFile      file(lightInfoPath);
  mINI::INIStructure ini;
  file.read(ini);

  //=======================
  // POINT LIGHTS
  //=======================
  int numOfPointLights = 0;
  if(ini.has("point"))
  {
    numOfPointLights = std::stoi(ini["point"]["count"]);
  }

  for(int i = 0; i < numOfPointLights; i++)
  {
    LightStructs::PointLight pointLightData;
    auto                     pointLightKey = "point-light-" + std::to_string(i);

    pointLightData.colour = {
        std::stof(ini[pointLightKey]["colr"]), std::stof(ini[pointLightKey]["colg"]), std::stof(ini[pointLightKey]["colb"]),
        std::stof(ini[pointLightKey]["cola"])  // intensity
    };

    pointLightData.position = {
        std::stof(ini[pointLightKey]["posx"]),
        std::stof(ini[pointLightKey]["posy"]),
        std::stof(ini[pointLightKey]["posz"]),
    };

    pointLightData.constantFactor    = std::stof(ini[pointLightKey]["constantfactor"]);
    pointLightData.linearFactor      = std::stof(ini[pointLightKey]["linearfactor"]);
    pointLightData.quadraticFactor   = std::stof(ini[pointLightKey]["quadraticfactor"]);
    pointLightData.isPointLightInUse = std::stof(ini[pointLightKey]["isinuse"]);

    scene.AddPointLight(&pointLightData);
  }

  //=======================
  // AREA LIGHTS
  //=======================

  int numOfAreaLights = 0;
  if(ini.has("area"))
  {
    numOfAreaLights = std::stoi(ini["area"]["count"]);
  }

  for(int i = 0; i < numOfAreaLights; i++)
  {
    LightStructs::AreaLight areaLightData;
    auto                    areaLightKey = "area-light-" + std::to_string(i);

    areaLightData.colour = {std::stof(ini[areaLightKey]["colr"]), std::stof(ini[areaLightKey]["colg"]),
                            std::stof(ini[areaLightKey]["colb"])};

    areaLightData.intensity = std::stof(ini[areaLightKey]["cola"]);

    areaLightData.position = {
        std::stof(ini[areaLightKey]["posx"]),
        std::stof(ini[areaLightKey]["posy"]),
        std::stof(ini[areaLightKey]["posz"]),
    };

    areaLightData.scale = {std::stof(ini[areaLightKey]["scalex"]), std::stof(ini[areaLightKey]["scaley"])};

    areaLightData.orientation = {std::stof(ini[areaLightKey]["quatX"]), std::stof(ini[areaLightKey]["quatY"]),
                                 std::stof(ini[areaLightKey]["quatZ"]), std::stof(ini[areaLightKey]["quatW"])};

    areaLightData.twoSided         = std::stoi(ini[areaLightKey]["twosided"]);
    areaLightData.isAreaLightInUse = std::stoi(ini[areaLightKey]["isInUse"]);


    scene.AddAreaLight(&areaLightData);
  }


  //=======================
  // DIRECTION LIGHT
  //=======================
  if(ini.has("directional"))
  {
    LightStructs::DirectionalLight directionalLightData;
    directionalLightData.colour = {
        std::stof(ini["directional"]["colr"]),
        std::stof(ini["directional"]["colg"]),
        std::stof(ini["directional"]["colb"]),
        std::stof(ini["directional"]["cola"]),
    };

    directionalLightData.direction = {
        std::stof(ini["directional"]["directionX"]),
        std::stof(ini["directional"]["directionY"]),
        std::stof(ini["directional"]["directionZ"]),
    };


    scene.AddDirectionalLight(&directionalLightData);
  }

  //=======================
  // SKY-BOX LIGHT
  //=======================
  if(ini.has("sky-box"))
  {
    LightStructs::EnvLight envLight;

    envLight.ambientIntensity = std::stof(ini["sky-box"]["ambient-intensity"]);
    envLight.hdrPath          = ini["sky-box"]["hdr"];

    scene.AddSkyBox(&envLight);
  }
}
