//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_PROJECT_HPP
#define VULKAN_RTX_PROJECT_HPP
#include "json.hpp"
#include "Application/Utils/Uuid.hpp"


#include <filesystem>

namespace ApplicationCore {

static const char* ASSET_TYPE_MESH     = "mesh";
static const char* ASSET_TYPE_MATERIAL = "material";
static const char* ASSET_TYPE_TEXTURE  = "texture";

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

    const char* JSON_ASSETS_BROWSER_SHOW_TEXTURES = "Assets browser show textures";
    bool        showTextures                      = true;
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


//===========================================================================

//=========================================================
// Assets database
// - main point of this class is to map assets to the correct paths
// - TODO: figure out how the changes of paths will be handled
// - when asset is imported from gltf or whatever, it is being assigned UUID which is used
// to look up the given asset from here
// - this class just returns abstract AssetEntry struct with all info that engine needs to load the asset from the file
//==========================================================
enum EAssetEntryType
{
    Mesh = 0,
    Material,
    Texture,
};
struct AssetEntry
{
    uuid::UUID      uuid;
    EAssetEntryType eType;  // same as type but not string for faster if statements
    std::string     type;   // "mesh", "material", "texture"

    // the path is being modified in the VAsset2 class where it constructs to correct path based on the directory, file name and path
    std::filesystem::path path;
    std::string           name;
};

class AssetsDatabase
{
  public:
    AssetsDatabase(const std::filesystem::path& projectPath);

    void        AddAsset(uuid::UUID uuid, AssetEntry& entry);
    void        RemoveAsset(uuid::UUID uuid);
    void        Save();
    AssetEntry  GetAsset(uuid::UUID uuid);
    AssetEntry& GetAsset(std::filesystem::path& path);
    AssetEntry& RequestNewAssetEntry(EAssetEntryType type, std::filesystem::path path, std::string name);
    std::string ExtensionFromType(EAssetEntryType type);

  private:
    const std::filesystem::path                           m_projectDbFile = "VAssets.json";
    std::filesystem::path                                 m_projectDbPath;
    std::unordered_map<uuid::UUID, AssetEntry>            m_assets;
    std::unordered_map<std::filesystem::path, uuid::UUID> m_pathToAsset;
};

//=========================================================
// Project class
// - responsible for handling the project (stores all paths, textures and whatnot
// - it is created before everything else and is being passed down to the application// - it contains instance of assets database used to retrieve any asset requested by the application
//==========================================================

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

    void        AddAsset(uuid::UUID uuid, AssetEntry& entry);
    void        RemoveAsset(uuid::UUID uuid);
    AssetEntry& GetAsset(uuid::UUID& uuid);
    AssetEntry& GetAsset(std::filesystem::path& path);

    AssetEntry& RequestAssetEntryAndRegister(EAssetEntryType type, std::filesystem::path path, std::string name);

  private:
    std::filesystem::path           m_projectPath;
    const std::filesystem::path     m_templatePath = "Resources/ProjectTemplate";
    ProjectConfig                   m_projectConfig;
    const std::filesystem::path     m_projectConfigFile = "VProject.json";
    std::unique_ptr<AssetsDatabase> m_assetsDatabase;

    void WriteProjectConfig(std::filesystem::path& path);
    void ReadProjectConfig(std::filesystem::path& path);
};


}  // namespace ApplicationCore

#endif  //VULKAN_RTX_PROJECT_HPP
