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


    //=======================================
    // CAMERA CONFIG
    //=======================================
    auto &cam = client.GetCamera();
    EngineConfig["Camera"].set({
        {"FOV", std::to_string(cam.GetFOV())},
        {"Near", std::to_string(cam.GetNearPlane()) },
        {"Far", std::to_string(cam.GetFarPlane())},
        {"Speed", std::to_string(cam.GetSpeed())},
    });

    //========================================
    // OTHERS
    //========================================
    EngineConfig["Other"].set({
        {"HasBeenSaved", std::to_string(GlobalVariables::hasSessionBeenSaved)}
    });

    //=======================================
    // UPDATE THE CONFIG
    //=======================================
    if (engineConfigFile.write(EngineConfig, true)) Utils::Logger::LogSuccessClient("Engine config saved");
    else Utils::Logger::LogErrorClient("Failed to save engine config! ");

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

    //=======================================
    // CAMERA CONFIG
    //=======================================
    auto &cam = client.GetCamera();
    if (EngineConfig.has("Camera"))
    {
        cam.GetFOV() = std::stof(EngineConfig["Camera"]["FOV"]);
        cam.GetNearPlane() = std::stof(EngineConfig["Camera"]["Near"]);
        cam.GetFarPlane() = std::stof(EngineConfig["Camera"]["Far"]);
        cam.GetSpeed() = std::stof(EngineConfig["Camera"]["Speed"]);
    }

    if (EngineConfig.has("Other"))
    {
        GlobalVariables::hasSessionBeenSaved = std::stoi(EngineConfig["Other"]["HasBeenSaved"]);
    }
}
