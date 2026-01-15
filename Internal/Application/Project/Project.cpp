//
// Created by simon on 14/01/2026.
//

#include "Project.hpp"

#include "Application/Logger/Logger.hpp"

#include <fstream>


namespace ApplicationCore {
Project::Project() = default;

void Project::PrintHelp()
{
    std::cout << "==========================================================\n";
    std::cout << "-h or --help : print this help message\n";
    std::cout << "-c or --create [path] --name [name] :creates new project with the name\n";
    std::cout << "-o or --open [path]/VProject.json: print this help message\n";
    std::cout << "==========================================================\n";
}

void Project::CrateNew(const std::filesystem::path& path, const std::string& name)
{
    //====================================
    // Configure all paths
    std::string projectName = name.empty() ? "VulkanRtx" : name;
    Utils::Logger::LogInfoCLI("Creating new project at path:    " + path.string());
    m_projectPath          = path / projectName;
    auto projectConfigPath = m_projectPath / "VProject.json";

    //=====================================
    // Create directory within the path
    std::filesystem::create_directories(m_projectPath);

    //========================================================
    // Copy template contents into the new project directory
    std::filesystem::copy(m_templatePath, m_projectPath,
                          std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    //========================================
    // Load the config file
    m_projectConfig.meta.projectName = name;
    Project::WriteProjectConfig(projectConfigPath);
}

void Project::OpenFrom(const std::filesystem::path& path)
{
    Utils::Logger::LogInfoCLI("Opening project from path :" + path.string());
    m_projectPath          = path;
    auto projectConfigPath = m_projectPath / "VProject.json";
    if(access(projectConfigPath.c_str(), F_OK) == -1)
    {
        Utils::Logger::LogErrorCLI("Project config VProject.json file not found at:" + projectConfigPath.string());
    }
    else
    {
        Utils::Logger::LogInfoCLI("Loading your project....");
        ReadProjectConfig(projectConfigPath);
    }
}
ProjectConfig& Project::GetProjectConfig()
{
    return m_projectConfig;
}
std::filesystem::path Project::GetProjectPath()
{
    return m_projectPath;
}

void Project::WriteProjectConfig(std::filesystem::path& path)
{
    json j;
    j[m_projectConfig.meta.JSON_FIELD_NAME] = {{m_projectConfig.meta.JSON_FIELD_PROJECT_NAME, m_projectConfig.meta.projectName}};

    std::ofstream file(path.string());
    file << j.dump(4);
}

void Project::ReadProjectConfig(std::filesystem::path& path)
{
    Utils::Logger::LogInfoCLI("Reading project config from path:" + path.string());
    std::ifstream file(path);
    if(!file.is_open())
        throw std::runtime_error("Project config file not found");

    json j;
    file >> j;
    ProjectConfig cfg;

    if(j.contains(cfg.meta.JSON_FIELD_NAME))
    {
        const auto& data     = j[cfg.meta.JSON_FIELD_NAME];
        cfg.meta.projectName = data.value(cfg.meta.JSON_FIELD_PROJECT_NAME, "Unknown project name");
    }

    Utils::Logger::LogInfoCLI("Done !");
    m_projectConfig = cfg;
}
}  // namespace ApplicationCore