//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_PROJECT_HPP
#define VULKAN_RTX_PROJECT_HPP
#include "json.hpp"


#include <filesystem>

namespace ApplicationCore {

using json = nlohmann::json;

//=================================================
// Struct defining how the project config will look
struct ProjectConfig
{
    struct Meta
    {
        const char* JSON_FIELD_NAME = "Meta";

        std::string projectName;
        const char* JSON_FIELD_PROJECT_NAME = "Project name";
    } meta;
};


class Project
{
  public:
    Project();
    void PrintHelp();
    void CrateNew(const std::filesystem::path& path, const std::string& name);
    void OpenFrom(const std::filesystem::path& path);

  private:
    ProjectConfig               projectConfig;
    std::filesystem::path       m_projectPath;
    const std::filesystem::path m_templatePath = "Resources/ProjectTemplate";
    ProjectConfig               m_projectConfig;

    void WriteProjectConfig(std::filesystem::path& path);
    void ReadProjectConfig(std::filesystem::path& path);
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_PROJECT_HPP
