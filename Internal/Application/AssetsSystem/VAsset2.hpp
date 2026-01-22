//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VFILE_HPP
#define VULKAN_RTX_VFILE_HPP

#include "Application/Project/Project.hpp"
#include "Application/Utils/Uuid.hpp"
#include "Application/VertexArray/VertexArray.hpp"


#include <any>
#include <filesystem>

namespace ApplicationCore {

template <typename Header>
class VAsset2
{
  public:
    VAsset2(AssetEntry& assetEntry, std::string fileType);
    virtual bool     Save(std::filesystem::path& path) = 0;
    virtual bool     Load()                            = 0;
    virtual std::any LoadData()                        = 0;

    uuid::UUID GetUUID();

  protected:
    Header                      m_fileHeader;
    std::filesystem::path       m_path;
    std::string                 m_fileType = ".VAsset";
    ApplicationCore::AssetEntry m_databaseEntry;


  protected:
    void SaveHeader(std::filesystem::path& path);
};


template <typename Header>
VAsset2<Header>::VAsset2(AssetEntry& assetEntry, std::string fileType)
    : m_databaseEntry(assetEntry)
    , m_fileType(fileType)
    , m_path(assetEntry.path)
{
}
template <typename Header>
uuid::UUID VAsset2<Header>::GetUUID()
{
    return m_databaseEntry.uuid;
}

template <typename Header>
void VAsset2<Header>::SaveHeader(std::filesystem::path& path)
{
}


}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VFILE_HPP
