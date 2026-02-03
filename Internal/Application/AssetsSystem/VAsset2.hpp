//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VFILE_HPP
#define VULKAN_RTX_VFILE_HPP

#include "Application/Logger/Logger.hpp"
#include "Application/Project/Project.hpp"
#include "Application/Utils/Uuid.hpp"
#include "Application/VertexArray/VertexArray.hpp"


#include <any>
#include <filesystem>
#include <fstream>

namespace ApplicationCore {

template <typename Header, typename VulkanHandle>
class VAsset2
{
  public:
    VAsset2(std::string fileType);
    VAsset2(AssetEntry& assetEntry, std::string fileType);
    virtual bool     Save(std::filesystem::path& path) = 0;
    virtual bool     Load()                            = 0;
    virtual std::any LoadData()                        = 0;

    uuid::UUID GetUUID();

    Header GetHeader();
    void   LoadHeader();
    void   SetVulkanHandle(std::unique_ptr<VulkanHandle> vulkanHandle);

  public:
    static Header ReadHeader(const std::filesystem::path& path);

  protected:
    Header                        m_fileHeader;
    std::filesystem::path         m_path;
    std::string                   m_fileType = ".VAsset";
    ApplicationCore::AssetEntry   m_databaseEntry;
    std::unique_ptr<VulkanHandle> m_vulkanHandle;

  protected:
    bool m_loaded = false;
    void SaveHeader(std::filesystem::path& path);
};


template <typename Header, typename VulkanHandle>
VAsset2<Header, VulkanHandle>::VAsset2(std::string fileType)
    : m_fileType(fileType)
{
    m_databaseEntry = {};
}

template <typename Header, typename VulkanHandle>
VAsset2<Header, VulkanHandle>::VAsset2(AssetEntry& assetEntry, std::string fileType)
    : m_databaseEntry(assetEntry)
    , m_fileType(fileType)
    , m_path(assetEntry.path)
{
}

template <typename Header, typename VulkanHandle>
uuid::UUID VAsset2<Header, VulkanHandle>::GetUUID()
{
    return m_databaseEntry.uuid;
}
template <typename Header, typename VulkanHandle>
Header VAsset2<Header, VulkanHandle>::GetHeader()
{

    return m_fileHeader;
}

template <typename Header, typename VulkanHandle>
void VAsset2<Header, VulkanHandle>::LoadHeader()
{
    if(m_loaded)
    {
        return;
    }
    std::ifstream input;
    input.open(m_path, std::ios::in | std::ios::binary);

    if(!input.good())
    {
        Utils::Logger::LogErrorClient("File " + m_path.string() + " is corrupted ! ");
    }

    m_fileHeader.Deserialize(input);

    if(input.fail())
    {
        switch(errno)
        {
            case EACCES:
                throw std::runtime_error("Error at: " + m_path.string() + " permission denied");
            case ENOENT:
                throw std::runtime_error("Error at: " + m_path.string() + " file not found");
            default:
                Utils::Logger::LogInfoVerboseOnlyClient("File: " + m_path.filename().string() + " procdues unknown error while reading  ! ");
        }
    }
    else
    {
        input.close();
    }
    m_loaded = true;
}

template <typename Header, typename VulkanHandle>
void VAsset2<Header, VulkanHandle>::SetVulkanHandle(std::unique_ptr<VulkanHandle> vulkanHandle)
{
    m_vulkanHandle = std::move(vulkanHandle);
}
template <typename Header, typename VulkanHandle>
Header VAsset2<Header, VulkanHandle>::ReadHeader(const std::filesystem::path& path)
{
    auto header = Header();

    std::ifstream input;
    input.open(path, std::ios::in | std::ios::binary);

    header.Deserialize(input);

    if(input.fail())
    {
        switch(errno)
        {
            case EACCES:
                throw std::runtime_error("Error while quick opening header at: " + path.string() + " permission denied");
            case ENOENT:
                throw std::runtime_error("Error while quick opening header at: " + path.string() + " file not found");
            default:
                Utils::Logger::LogInfoVerboseOnlyClient("File: " + path.filename().string() + " read successfully ! ");
        }
    }

    return header;
}

template <typename Header, typename VulkanHandle>
void VAsset2<Header, VulkanHandle>::SaveHeader(std::filesystem::path& path)
{
}

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VFILE_HPP
