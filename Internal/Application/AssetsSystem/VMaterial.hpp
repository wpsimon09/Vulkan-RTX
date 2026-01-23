//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VMATERIAL_HPP
#define VULKAN_RTX_VMATERIAL_HPP
#include "FileHeaders.hpp"
#include "VAsset2.hpp"

namespace ApplicationCore {

// IDK yet what will be the material's Vulkan header so i will use just int for that for now
class VMaterial : public VAsset2<VMaterialHeader, int>
{
  public:
    VMaterial(AssetEntry& databaseEntry);
    VMaterial(AssetEntry& databaseEntry, VMaterialHeader& materialData);
    bool     Save(std::filesystem::path& path) override;
    bool     Load() override;
    std::any LoadData() override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VMATERIAL_HPP
