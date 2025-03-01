//
// Created by wpsimon09 on 01/03/25.
//
#include "GlobalVariables.hpp"
#include "tINI/ini.h"
#include "Application/Client.hpp"
#include "Editor/UIContext/UIContext.hpp"


void GlobalVariables::SaveConfig(Client& client, VEditor::UIContext& uiContext)
{
    const std::string engineConfigPath = configFolder / "EngineConfig.ini";
    mINI::INIFile engineConfigFile(engineConfigPath);

    //===================================
    // engine related
    //===================================
    mINI::INIStructure engineConfig;
    auto &cam = client.GetCamera();
    engineConfig["Camera"]["FOV"] = cam.GetFOV();
    engineConfig["Camera"]["Far"] = cam.GetFarPlane();
    engineConfig["Camera"]["Near"] = cam.GetNearPlane();
    engineConfig["Camera"]["Speed"] = cam.GetSpeed();

    if (engineConfigFile.generate(engineConfig, true)) Utils::Logger::LogSuccessClient("Engine config saved");
    else Utils::Logger::LogErrorClient("Failed to save engine config! ");


    //=====================================
    // LIGHT DATA GOES TO ANOTHER INI FILE
    //=====================================
    const std::string lightInfoPath = configFolder / "LightInfo.ini";
    mINI::INIFile lightInfoFile(lightInfoPath);



}

void GlobalVariables::LoadConfig()
{
}
