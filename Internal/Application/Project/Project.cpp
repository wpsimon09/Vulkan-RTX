//
// Created by simon on 14/01/2026.
//

#include "Project.hpp"

#include "Application/AssetsSystem/VAsset2.hpp"
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
AssetEntry& Project::GetAsset(uuid::UUID& uuid)
{
    m_assetsDatabase->GetAsset(uuid);
}

AssetEntry& Project::GetAsset(std::filesystem::path& path)
{
    return m_assetsDatabase->GetAsset(path);
}

AssetEntry& Project::RequestAssetEntryAndRegister(EAssetEntryType type, std::filesystem::path path, std::string name)
{
    return m_assetsDatabase->RequestNewAssetEntry(type, path, name);
}
void Project::Reindex()
{
    m_assetsDatabase->Reindex();
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
                       {ec.JSON_ASSETS_BROWSER_SHOW_ICONS, ec.showIcons},
                       {ec.JSON_ASSETS_BROWSER_SHOW_TEXTURES, ec.showTextures}};

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

        cfg.editorConfig.AssetBrowserIconSize = data.value(cfg.editorConfig.JSON_ASSETS_BROWSER_ICON_SIZE, 55);

        cfg.editorConfig.TileSize = data.value(cfg.editorConfig.JSON_ASSETS_BROWSER_TILE_SIZE, 20);

        cfg.editorConfig.IconSpacing = data.value(cfg.editorConfig.JSON_ASSETS_BROWSER_ICON_SPACING, 10);

        cfg.editorConfig.Columns = data.value(cfg.editorConfig.JSON_ASSETS_BROWSER_COLUMNS, 3);

        cfg.editorConfig.showIcons = data.value(cfg.editorConfig.JSON_ASSETS_BROWSER_SHOW_ICONS, true);

        cfg.editorConfig.showTextures = data.value(cfg.editorConfig.JSON_ASSETS_BROWSER_SHOW_TEXTURES, true);
    }

    Utils::Logger::LogInfoCLI("Done !");
    m_projectConfig = cfg;
}


//=====================================================
// ------------------- Assets database ----------------
//=====================================================

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
            entry.type                = value["type"];
            entry.path                = value["path"].get<std::string>();
            entry.name                = value["name"];
            entry.uuid                = uuidStr;
            entry.eType               = static_cast<EAssetEntryType>(value["typeE"]);
            m_assets[uuidStr]         = entry;
            m_pathToAsset[entry.path] = uuidStr;
        }
    }
}

void AssetsDatabase::AddAsset(uuid::UUID uuid, AssetEntry& entry)
{
    m_assets[uuid]            = entry;
    m_pathToAsset[entry.path] = uuid;
}

void AssetsDatabase::RemoveAsset(uuid::UUID uuid)
{
    auto& path = m_assets.at(uuid).path;
    m_pathToAsset.erase(path);
    m_assets.erase(uuid);
}

void AssetsDatabase::Save()
{
    json j;
    for(auto& [uuidStr, value] : m_assets)
    {
        j[uuidStr] = {
            {"type", value.type}, {"typeE", static_cast<int>(value.eType)}, {"path", value.path}, {"name", value.name}};
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

AssetEntry& AssetsDatabase::GetAsset(std::filesystem::path& path)
{
    assert(m_pathToAsset.contains(path) && "Asset was not found");
    auto uuid = m_pathToAsset.at(path);
    return m_assets[uuid];
}

AssetEntry& AssetsDatabase::RequestNewAssetEntry(EAssetEntryType type, std::filesystem::path path, std::string name)
{
    std::string typeStr;
    switch(type)
    {
        case EAssetEntryType::Material: {
            typeStr = ASSET_TYPE_MATERIAL;
            break;
        }
        case EAssetEntryType::Texture: {
            typeStr = ASSET_TYPE_TEXTURE;
            break;
        }
        case EAssetEntryType::Mesh: {
            typeStr = ASSET_TYPE_MESH;
            break;
        }
    }

    path.replace_extension(ExtensionFromType(type));
    auto newAsset = AssetEntry{uuid::generate_uuid_v4(), type, typeStr, path, name};
    m_assets.insert({newAsset.uuid, newAsset});
    m_pathToAsset[path] = newAsset.uuid;
    return m_assets[newAsset.uuid];
}
std::string AssetsDatabase::ExtensionFromType(EAssetEntryType type)
{
    switch(type)
    {
        case EAssetEntryType::Material: {
            return ".VMat";
        }
        case EAssetEntryType::Texture: {
            return ".VTex";
        }
        case EAssetEntryType::Mesh: {
            return ".VMesh";
        }
        default:
            return "Unknown asset type";
    }
}
EAssetEntryType AssetsDatabase::TypeFromExtension(std::string extension)
{
    if(extension == ".VMat")
    {
        return EAssetEntryType::Material;
    }
    else if(extension == ".VTex")
    {
        return EAssetEntryType::Texture;
    }
    else if(extension == ".VMesh")
    {
        return EAssetEntryType::Mesh;
    }
    else
    {
        return EAssetEntryType::Unknown;
    }
}
void AssetsDatabase::Reindex()
{
    m_assets.clear();
    m_pathToAsset.clear();

    ParseDirectoryStructure(m_projectDbPath);
}
void AssetsDatabase::ParseDirectoryStructure(std::filesystem::path path)
{
    for(const auto& directory : std::filesystem::directory_iterator(path))
    {
        if(!directory.is_directory())
        {
            AssetEntry entry;
            entry.path  = directory.path();
            entry.eType = TypeFromExtension(directory.path().extension().string());
            entry.type  = ExtensionFromType(entry.eType);
            if(entry.eType == Unknown)
                continue;
            switch(entry.eType)
            {
                case EAssetEntryType::Material: {
                    auto header = VAsset2<VMaterialHeader, int>::ReadHeader(directory.path());
                    entry.uuid  = header.uuid;
                    entry.name  = header.name;
                    break;
                }
                case EAssetEntryType::Texture: {
                    auto header = VAsset2<VTextureHeader, int>::ReadHeader(directory.path());
                    entry.uuid  = header.uuid;
                    entry.name  = header.name;
                    break;
                }
                case EAssetEntryType::Mesh: {
                    auto header = VAsset2<VMeshHeader, int>::ReadHeader(directory.path());
                    entry.uuid  = header.uuid;
                    entry.name  = header.name;
                    break;
                }
            }
            m_assets.insert({entry.uuid, entry});
            m_pathToAsset.insert({entry.path, entry.uuid});
        }
        else
        {
            ParseDirectoryStructure(directory.path());
        }
    }
}

}  // namespace ApplicationCore