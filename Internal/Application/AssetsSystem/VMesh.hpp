//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VMESH_HPP
#define VULKAN_RTX_VMESH_HPP
#include "FileHeaders.hpp"
#include "VAsset2.hpp"

namespace ApplicationCore {


class VMesh : public VAsset2<VMeshHeader>
{
  public:
    VMesh(AssetEntry& databaseEntry);
    VMesh(AssetEntry& databaseEntry, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    bool     Save(std::filesystem::path& path) override;
    bool     Load() override;
    std::any LoadData() override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VMESH_HPP
