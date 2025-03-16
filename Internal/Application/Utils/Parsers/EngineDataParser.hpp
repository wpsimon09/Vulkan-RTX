//
// Created by wpsimon09 on 02/03/25.
//

#ifndef ENGINEDATAPARSER_HPP
#define ENGINEDATAPARSER_HPP
#include "tINI/ini.h"
#include "Vulkan/Global/GlobalVariables.hpp"

namespace VEditor
{
    class UIContext;
}

class Client;
namespace ApplicationCore
{
    void SaveConfig(Client& client, VEditor::UIContext& uiContext);
    void LoadConfig(Client& client, VEditor::UIContext& uiContext);
    void LoadConfig();

    const std::filesystem::path engineConfigPath = GlobalVariables::configFolder / "EngineConfig.ini";
    inline mINI::INIStructure EngineConfig;

}

#endif //ENGINEDATAPARSER_HPP
