//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_PROJECT_HPP
#define VULKAN_RTX_PROJECT_HPP
#include "json.hpp"
#include "Application/Utils/Uuid.hpp"


#include <filesystem>

namespace ApplicationCore {

using json = nlohmann::json;

//=================================================
// Struct defining how the project config will look
struct EditorConfig
{
    const char* JSON_NAME = "Assets browser config";

    const char* JSON_ASSETS_BROWSER_ICON_SIZE = "Assets browser icon size";
    float       AssetBrowserIconSize          = 56.0f;

    const char* JSON_ASSETS_BROWSER_TILE_SIZE = "Assets browser tile size";
    float       TileSize                      = 64.0f;

    const char* JSON_ASSETS_BROWSER_ICON_SPACING = "Assets browser icon spacing";
    float       IconSpacing                      = 12.0f;

    const char* JSON_ASSETS_BROWSER_COLUMNS = "Assets browser columns";
    int         Columns                     = 1;

    const char* JSON_ASSETS_BROWSER_SHOW_ICONS = "Assets browser show icons";
    bool        showIcons                      = true;
};

struct Meta
{
    const char* JSON_FIELD_NAME = "Meta";

    std::string projectName;
    const char* JSON_FIELD_PROJECT_NAME = "Project name";
};

struct ProjectConfig
{
    EditorConfig editorConfig;
    Meta         meta;
};


class Project
{
  public:
    Project();
    static void PrintHelp();
    void        CrateNew(const std::filesystem::path& path, const std::string& name);
    void        OpenFrom(const std::filesystem::path& path);

    ProjectConfig&        GetProjectConfig();
    std::filesystem::path GetProjectPath();
    void                  End();

  private:
    std::filesystem::path       m_projectPath;
    const std::filesystem::path m_templatePath = "Resources/ProjectTemplate";
    ProjectConfig               m_projectConfig;
    const std::filesystem::path m_projectConfigFile = "VProject.json";


    void WriteProjectConfig(std::filesystem::path& path);
    void ReadProjectConfig(std::filesystem::path& path);
};

struct AssetEntry
{
    std::string           type;  // "mesh", "material", "texture"
    std::filesystem::path path;
    std::string           name;
    uuid::UUID            materialUUID;
};

class AssetsDatabase
{
  public:
    AssetsDatabase(std::filesystem::path& projectPath);

    void AddAsset(uuid::UUID uuid, AssetEntry& entry);
    void RemoveAsset(uuid::UUID uuid);

  private:
    std::unordered_map<uuid::UUID, AssetEntry> m_assets;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_PROJECT_HPP
