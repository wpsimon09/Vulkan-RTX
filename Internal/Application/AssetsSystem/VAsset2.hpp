//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VFILE_HPP
#define VULKAN_RTX_VFILE_HPP

#include "Application/Utils/Uuid.hpp"


#include <any>
#include <filesystem>

namespace ApplicationCore {

template <typename Header>
class VAsset2
{
  public:
    VAsset2(std::string name, std::string fileType);
    virtual bool     Save(std::filesystem::path& path);
    virtual bool     Load();
    virtual std::any LoadData();

  protected:
    Header                m_fileHeader;
    uuid::UUID            m_uuid;
    std::filesystem::path m_path;
    std::string           m_name     = "unknonw";
    std::string           m_fileType = ".VAsset";
};
template <typename Header>
VAsset2<Header>::VAsset2(std::string name, std::string fileType)
    : m_name(name)
    , m_fileType(fileType)
{
}

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VFILE_HPP
