//
// Created by wpsimon09 on 02/03/25.
//
#include "EngineDataParser.hpp"


#include "tINI/ini.h"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Client.hpp"
#include "Application/Rendering/Camera/Camera.hpp"

void ApplicationCore::SaveConfig(Client& client, VEditor::UIContext& uiContext)
{

    mINI::INIFile engineConfigFile(engineConfigPath);

    //===================================
    // engine related
    //===================================
    mINI::INIStructure engineConfig;
    auto &cam = client.GetCamera();
    engineConfig["Camera"]["FOV"] =   std::to_string(cam.GetFOV());
    engineConfig["Camera"]["Far"] =   std::to_string(cam.GetFarPlane());
    engineConfig["Camera"]["Near"] =  std::to_string(cam.GetNearPlane());
    engineConfig["Camera"]["Speed"] = std::to_string(cam.GetSpeed());

    if (engineConfigFile.write(engineConfig, true)) Utils::Logger::LogSuccessClient("Engine config saved");
    else Utils::Logger::LogErrorClient("Failed to save engine config! ");


    //=====================================
    // LIGHT DATA GOES TO ANOTHER INI FILE
    //=====================================
    const std::string lightInfoPath = GlobalVariables::configFolder / "LightInfo.ini";
    mINI::INIFile lightInfoFile(lightInfoPath);

}

void ApplicationCore::LoadConfig(Client& client, VEditor::UIContext& uiContext)
{
    mINI::INIFile engineConfigFile(engineConfigPath);
    if (engineConfigFile.read(EngineConfig)){
        Utils::Logger::LogSuccessClient("Engine config read successfully");
    }else
    {
        Utils::Logger::LogErrorClient("Engine config does not exist creating new one...");

        assert(engineConfigFile.generate(EngineConfig, true) == true);
    }
}
