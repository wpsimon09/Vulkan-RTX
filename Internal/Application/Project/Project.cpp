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
    //===================================
    // create assets database
    m_assetsDatabase = std::make_unique<AssetsDatabase>(path);

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
    m_assetsDatabase->Save();
}

void Project::OpenFrom(const std::filesystem::path& path)
{
    Utils::Logger::LogInfoCLI("Opening project from path :" + path.string());
    m_projectPath          = path;
    auto projectConfigPath = m_projectPath / m_projectConfigFile;
    if(access(projectConfigPath.c_str(), F_OK) == -1)
    {
        Utils::Logger::LogErrorCLI("Project config VProject.json file not found at:" + projectConfigPath.string());
        throw std::runtime_error("Project config VProject.json file not found, create new project with --create [project]");
    }
    else
    {
        Utils::Logger::LogInfoCLI("Loading your project....");
        ReadProjectConfig(projectConfigPath);
    }

    //===================================
    // create assets database
    m_assetsDatabase = std::make_unique<AssetsDatabase>(path);
}
ProjectConfig& Project::GetProjectConfig()
{
    return m_projectConfig;
}
std::filesystem::path Project::GetProjectPath()
{
    return m_projectPath;
}

void Project::End()
{
    auto projectConfigPath = m_projectPath / m_projectConfigFile;
    WriteProjectConfig(projectConfigPath);
    m_assetsDatabase->Save();
}
void Project::AddAsset(uuid::UUID uuid, AssetEntry& entry)
{
    m_assetsDatabase->AddAsset(uuid, entry);
}

void Project::RemoveAsset(uuid::UUID uuid)
{
    m_assetsDatabase->RemoveAsset(uuid);
}
AssetEntry Project::GetAsset(uuid::UUID uuid)
{
    m_assetsDatabase->GetAsset(uuid);
}

AssetEntry& Project::RequestAssetEntryAndRegister(std::string type, std::filesystem::path path, std::string name)
{
    auto& assetEntiry = m_assetsDatabase->RequestNewAssetEntry(type, path, name);
}

void Project::WriteProjectConfig(std::filesystem::path& path)
{
    //=================================
    // Meta data about project
    json  j;
    auto& m                                 = m_projectConfig.meta;
    j[m_projectConfig.meta.JSON_FIELD_NAME] = {{m.JSON_FIELD_PROJECT_NAME, m.projectName}};

    //================================
    // Editor config
    auto& ec        = m_projectConfig.editorConfig;
    j[ec.JSON_NAME] = {{ec.JSON_ASSETS_BROWSER_ICON_SIZE, ec.AssetBrowserIconSize},
                       {ec.JSON_ASSETS_BROWSER_TILE_SIZE, ec.TileSize},
                       {ec.JSON_ASSETS_BROWSER_ICON_SPACING, ec.IconSpacing},
                       {ec.JSON_ASSETS_BROWSER_COLUMNS, ec.Columns},
                       {ec.JSON_ASSETS_BROWSER_SHOW_ICONS, ec.showIcons}};

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

    if(j.contains(cfg.editorConfig.JSON_NAME))
    {
        const auto& data = j[cfg.editorConfig.JSON_NAME];

        cfg.editorConfig.AssetBrowserIconSize = data[cfg.editorConfig.JSON_ASSETS_BROWSER_ICON_SIZE];

        cfg.editorConfig.TileSize = data[cfg.editorConfig.JSON_ASSETS_BROWSER_TILE_SIZE];

        cfg.editorConfig.IconSpacing = data[cfg.editorConfig.JSON_ASSETS_BROWSER_ICON_SPACING];

        cfg.editorConfig.Columns = data[cfg.editorConfig.JSON_ASSETS_BROWSER_COLUMNS];

        cfg.editorConfig.showIcons = data[cfg.editorConfig.JSON_ASSETS_BROWSER_SHOW_ICONS];
    }

    Utils::Logger::LogInfoCLI("Done !");
    m_projectConfig = cfg;
}

AssetsDatabase::AssetsDatabase(const std::filesystem::path& projectPath)
    : m_projectDbPath(projectPath)
{
    //open the file    Utils::Logger::LogInfoCLI("Reading project config from path:" + path.string());
    const auto&   dbPath = m_projectDbPath / m_projectDbFile;
    std::ifstream file(dbPath);
    if(file.is_open())
    {
        json j;
        file >> j;

        for(auto& [uuidStr, value] : j.items())
        {
            AssetEntry entry;
            entry.type = value["type"];
            entry.path = value["path"].get<std::string>();
            entry.name = value["name"];
            if(value.contains("material"))
                entry.materialUUID = value["material"];
            m_assets[uuidStr] = entry;
        }
    }
}

void AssetsDatabase::AddAsset(uuid::UUID uuid, AssetEntry& entry)
{
    m_assets[uuid] = entry;
}

void AssetsDatabase::RemoveAsset(uuid::UUID uuid)
{
    m_assets.erase(uuid);
}

void AssetsDatabase::Save()
{
    json j;
    for(auto& [uuidStr, value] : m_assets)
    {
        j[uuidStr] = {{"type", value.type}, {"path", value.path}, {"name", value.name}, {"material_id", value.materialUUID}};
    }

    std::ofstream file(m_projectDbPath / m_projectDbFile);
    if(!file.is_open())
        throw std::runtime_error("Project db file not found");

    file << j.dump(4);
}  // namespace ApplicationCore
AssetEntry AssetsDatabase::GetAsset(uuid::UUID uuid)
{
    assert(m_assets.contains(uuid) && "Asset was not found");
    return m_assets[uuid];
}

AssetEntry& AssetsDatabase::RequestNewAssetEntry(std::string type, std::filesystem::path path, std::string name)
{
    auto newAsset = {uuid::generate_uuid_v4(), type, path, name, ""};
    m_assets.insert({newAsset.uuid, newAsset});
    return m_assets[newAsset.uuid];
}

}  // namespace ApplicationCore