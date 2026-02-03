//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VTEXTURE_HPP
#define VULKAN_RTX_VTEXTURE_HPP
#include "FileHeaders.hpp"
#include "VAsset2.hpp"

namespace ApplicationCore {

class VTexture : public VAsset2<VTextureHeader, VulkanCore::VImage2>
{
  public:
    VTexture();
    /**
     *  Loads the texture from the .VTex file
     * @param databaseEntry asset databse entry
     */
    VTexture(AssetEntry& databaseEntry);

    /**
   *  Creates a new .VTex file and saves it to the disk
   * @param databaseEntry databse entry requested from assets database
   * @param header file header
   * @param pixels pixel data
   * @param size size of the pixels (bytes) 
   */
    VTexture(AssetEntry& databaseEntry, VTextureHeader& header, void* pixels, size_t size);
    bool     Save(std::filesystem::path& path) override;
    bool     Load() override;
    std::any LoadData() override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VTEXTURE_HPP
