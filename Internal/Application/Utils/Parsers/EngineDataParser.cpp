//
// Created by wpsimon09 on 02/03/25.
//
#include "EngineDataParser.hpp"


#include "tINI/ini.h"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Client.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Editor/EditorOptions.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/EngineOptions.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Application/ApplicationState/ApplicationState.hpp"
#include "Application/Structs/ParameterStructs.hpp"

void ApplicationCore::SaveConfig(Client& client, VEditor::UIContext& uiContext)
{

    mINI::INIFile engineConfigFile(engineConfigPath);


    //=======================================
    // CAMERA CONFIG
    //=======================================
    auto& cam = client.GetCamera();
    EngineConfig["Camera"].set({
        {"FOV", std::to_string(cam.GetFOV())},
        {"Near", std::to_string(cam.GetNearPlane())},
        {"Far", std::to_string(cam.GetFarPlane())},
        {"Speed", std::to_string(cam.GetSpeed())},
        //RAY TRACING
        {"Focal plane", std::to_string(cam.GetFocalLength())},
    });

    //=========================================
    // ENGINE CONFIG
    //=========================================
    EngineConfig["Engine"].set({
        {"Vertex buffer chunk size", std::to_string(GlobalVariables::EngineOptions::VertexBufferChunkSize)},
        {"Index buffer chunk size", std::to_string(GlobalVariables::EngineOptions::IndexBufferChunkSize)},

    });

    //=========================================
    // EDITOR CONFIG
    //=========================================
    EngineConfig["Editor"].set({
        {"Theme", std::to_string(GlobalVariables::EditorOptions::Theme)},
    });

    //=========================================
    // RENDERING CONFIG
    //=========================================
    EngineConfig["Rendering"].set(
        {{"Frustrum culling", std::to_string(GlobalVariables::RenderingOptions::EnableFrustrumCulling)},
         {"Depth-PrePass", std::to_string(GlobalVariables::RenderingOptions::PreformDepthPrePass)},
         {"ShadowMapPass", std::to_string(GlobalVariables::RenderingOptions::PreformShadowPass)},
         {"MSAAEnabled", std::to_string(GlobalState::MSAA)},
         {"MSAASamples", std::to_string(GlobalVariables::RenderingOptions::MSAASamples)},
         {"OutlineWidth", std::to_string(GlobalVariables::RenderingOptions::OutlineWidth)},
         {"Max recursion", std::to_string(GlobalVariables::RenderingOptions::MaxRecursionDepth)},
         {"Exposure", std::to_string(client.GetApplicationState().GetToneMappingParameters().exposure)},
         {"Gamma", std::to_string(client.GetApplicationState().GetToneMappingParameters().gamma)},
         {"Rays per pixel", std::to_string(GlobalVariables::RenderingOptions::RaysPerPixel)}});
    ;

    //=========================================
    // APPLICATION CONFIG
    //=========================================
    EngineConfig["Application"].set({
        {"Log limit", std::to_string(GlobalState::LogLimit)},
    });


    //========================================
    // OTHERS
    //========================================
    EngineConfig["Other"].set({{"HasBeenSaved", std::to_string(GlobalVariables::hasSessionBeenSaved)}});

    //=======================================
    // UPDATE THE CONFIG
    //=======================================
    if(engineConfigFile.write(EngineConfig, true))
        Utils::Logger::LogSuccessClient("Engine config saved");
    else
        Utils::Logger::LogErrorClient("Failed to save engine config! ");
}

void ApplicationCore::LoadClientSideConfig(Client& client, VEditor::UIContext& uiContext)
{
    mINI::INIFile engineConfigFile(engineConfigPath);

    if(engineConfigFile.read(EngineConfig))
    {
        Utils::Logger::LogSuccessClient("Engine config read successfully");
    }
    else
    {
        Utils::Logger::LogErrorClient("Engine config does not exist, creating new one...");
        assert(engineConfigFile.generate(EngineConfig, true) == true);
    }

    //=======================================
    // CAMERA CONFIG
    //=======================================
    auto& cam = client.GetCamera();
    if(EngineConfig.has("Camera"))
    {
        if(EngineConfig["Camera"].has("FOV"))
            cam.GetFOV() = (std::stof(EngineConfig["Camera"]["FOV"]));
        if(EngineConfig["Camera"].has("Near"))
            cam.GetNearPlane() = (std::stof(EngineConfig["Camera"]["Near"]));
        if(EngineConfig["Camera"].has("Far"))
            cam.GetFarPlane() = (std::stof(EngineConfig["Camera"]["Far"]));
        if(EngineConfig["Camera"].has("Speed"))
            cam.GetSpeed() = (std::stof(EngineConfig["Camera"]["Speed"]));
        if(EngineConfig["Camera"].has("Focal plane"))
            cam.GetFocalLength() = (std::stof(EngineConfig["Camera"]["Focal plane"]));

        if(EngineConfig["Rendering"].has("Exposure"))
            client.GetApplicationState().GetToneMappingParameters().exposure =
                (std::stof(EngineConfig["Rendering"]["Exposure"]));
        if(EngineConfig["Rendering"].has("Gamma"))
            client.GetApplicationState().GetToneMappingParameters().gamma = (std::stof(EngineConfig["Rendering"]["Gamma"]));
        cam.Recalculate();
    }
}

void ApplicationCore::LoadConfig()
{
    mINI::INIFile engineConfigFile(engineConfigPath);

    if(engineConfigFile.read(EngineConfig))
    {
        Utils::Logger::LogSuccessClient("Engine config read successfully");
    }
    else
    {
        Utils::Logger::LogErrorClient("Engine config does not exist, creating new one...");
        assert(engineConfigFile.generate(EngineConfig, true) == true);
    }

    //=======================================
    // ENGINE CONFIG
    //=======================================
    if(EngineConfig.has("Engine"))
    {
        if(EngineConfig["Engine"].has("Vertex buffer chunk size"))
            GlobalVariables::EngineOptions::VertexBufferChunkSize = std::stoi(EngineConfig["Engine"]["Vertex buffer chunk size"]);
        if(EngineConfig["Engine"].has("Index buffer chunk size"))
            GlobalVariables::EngineOptions::IndexBufferChunkSize = std::stoi(EngineConfig["Engine"]["Index buffer chunk size"]);
    }

    //=======================================
    // RENDERING CONFIG
    //=======================================
    if(EngineConfig.has("Rendering"))
    {
        if(EngineConfig["Rendering"].has("Frustrum culling"))  // Fixed key name
            GlobalVariables::RenderingOptions::EnableFrustrumCulling = std::stoi(EngineConfig["Rendering"]["Frustrum culling"]);

        if(EngineConfig["Rendering"].has("MSAAEnabled"))
            GlobalState::MSAA = std::stoi(EngineConfig["Rendering"]["MSAAEnabled"]);
        if(EngineConfig["Rendering"].has("MSAASamples"))
            GlobalVariables::RenderingOptions::MSAASamples = std::stoi(EngineConfig["Rendering"]["MSAASamples"]);
        if(EngineConfig["Rendering"].has("OutlineWidth"))
            GlobalVariables::RenderingOptions::OutlineWidth = std::stof(EngineConfig["Rendering"]["OutlineWidth"]);
        if(EngineConfig["Rendering"].has("Depth-PrePass"))
            GlobalVariables::RenderingOptions::PreformDepthPrePass =
                static_cast<bool>(std::stoi(EngineConfig["Rendering"]["Depth-PrePass"]));
        if(EngineConfig["Rendering"].has("ShadowMapPass"))
            GlobalVariables::RenderingOptions::PreformDepthPrePass =
                static_cast<bool>(std::stoi(EngineConfig["Rendering"]["ShadowMapPass"]));
        if(EngineConfig["Rendering"].has("Max recursion"))
            GlobalVariables::RenderingOptions::MaxRecursionDepth = (std::stoi(EngineConfig["Rendering"]["Max recursion"]));
        if(EngineConfig["Rendering"].has("Rays per pixel"))
            GlobalVariables::RenderingOptions::RaysPerPixel = (std::stoi(EngineConfig["Rendering"]["Rays per pixel"]));
    }

    //=======================================
    // APPLICATION CONFIG
    //=======================================
    if(EngineConfig.has("Application"))
    {
        if(EngineConfig["Application"].has("Log limit"))  // Fixed key name
            GlobalState::LogLimit = std::stoi(EngineConfig["Application"]["Log limit"]);
    }

    //=======================================
    // EDITOR SETTINGS
    //=======================================
    if(EngineConfig.has("Editor"))
    {
        if(EngineConfig["Editor"].has("Theme"))  // Fixed key name
            GlobalVariables::EditorOptions::Theme = static_cast<ETheme>(std::stoi(EngineConfig["Editor"]["Theme"]));
    }

    //=======================================
    // OTHER SETTINGS
    //=======================================
    if(EngineConfig.has("Other"))
    {
        if(EngineConfig["Other"].has("HasBeenSaved"))
            GlobalVariables::hasSessionBeenSaved = std::stoi(EngineConfig["Other"]["HasBeenSaved"]);
    }
}
