//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VTEXTURE_HPP
#define VULKAN_RTX_VTEXTURE_HPP
#include "FileHeaders.hpp"
#include "VAsset2.hpp"

namespace ApplicationCore {

class VTexture : public VAsset2<VTextureHeader>
{
  public:
    VTexture(AssetEntry& databaseEntry);
    VTexture(AssetEntry& databaseEntry, VTextureHeader& header, void* pixels, size_t size);
    bool     Save(std::filesystem::path& path) override;
    bool     Load() override;
    std::any LoadData() override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VTEXTURE_HPP
