//
// Created by wpsimon09 on 02/03/25.
//
#include "EngineDataParser.hpp"


#include "tINI/ini.h"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Client.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"

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

    //=========================================
    // RENDERING CONFIG
    //=========================================
    EngineConfig["Rendering"].set({
        {"Frustrum culling", std::to_string(GlobalVariables::RenderingOptions::EnableFrustrumCulling)},
        {"MSAAEnabled", std::to_string(GlobalState::MSAA)},
        {"MSAASamples", std::to_string(GlobalVariables::RenderingOptions::MSAASamples)}
    });

    //=========================================
    // APPLICATION CONFIG
    //=========================================
    EngineConfig["Application"].set({
            {"Log limit", std::to_string(GlobalState::LogLimit)},
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

    if (engineConfigFile.read(EngineConfig)) {
        Utils::Logger::LogSuccessClient("Engine config read successfully");
    } else {
        Utils::Logger::LogErrorClient("Engine config does not exist, creating new one...");
        assert(engineConfigFile.generate(EngineConfig, true) == true);
    }

    //=======================================
    // CAMERA CONFIG
    //=======================================
    auto &cam = client.GetCamera();
    if (EngineConfig.has("Camera"))
    {
        if (EngineConfig["Camera"].has("FOV")) cam.GetFOV() = (std::stof(EngineConfig["Camera"]["FOV"]));
        if (EngineConfig["Camera"].has("Near")) cam.GetNearPlane() = (std::stof(EngineConfig["Camera"]["Near"]));
        if (EngineConfig["Camera"].has("Far")) cam.GetFarPlane() = (std::stof(EngineConfig["Camera"]["Far"]));
        if (EngineConfig["Camera"].has("Speed")) cam.GetSpeed() = (std::stof(EngineConfig["Camera"]["Speed"]));

        cam.Recalculate();
    }

    //=======================================
    // RENDERING CONFIG
    //=======================================
    if (EngineConfig.has("Rendering"))
    {
        if (EngineConfig["Rendering"].has("FrustrumCulling"))
            GlobalVariables::RenderingOptions::EnableFrustrumCulling = std::stoi(EngineConfig["Rendering"]["FrustrumCulling"]);

        if (EngineConfig["Rendering"].has("MSAAEnabled"))
            GlobalState::MSAA = std::stoi(EngineConfig["Rendering"]["MSAAEnabled"]);

        if (EngineConfig["Rendering"].has("MSAASamples"))
            GlobalVariables::RenderingOptions::MSAASamples = std::stoi(EngineConfig["Rendering"]["MSAASamples"]);
    }

    //=======================================
    // APPLICATION CONFIG
    //=======================================
    if (EngineConfig.has("Application"))
    {
        if (EngineConfig["Application"].has("LogLimit"))
            GlobalState::LogLimit = std::stoi(EngineConfig["Application"]["LogLimit"]);
    }

    //=======================================
    // OTHER SETTINGS
    //=======================================
    if (EngineConfig.has("Other"))
    {
        if (EngineConfig["Other"].has("HasBeenSaved"))
            GlobalVariables::hasSessionBeenSaved = std::stoi(EngineConfig["Other"]["HasBeenSaved"]);
    }
}

