//
// Created by simon on 18/01/2026.
//

#include "VTexture.hpp"

#include <fstream>
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace ApplicationCore {
VTexture::VTexture(AssetEntry& databaseEntry)
    : VAsset2<ApplicationCore::VTextureHeader, VulkanCore::VImage2>(databaseEntry, ".VTex")
{
}
VTexture::VTexture(AssetEntry& databaseEntry, VTextureHeader& header, void* pixels, size_t size)
    : VAsset2<ApplicationCore::VTextureHeader, VulkanCore::VImage2>(databaseEntry, ".VTex")
{
    // since it would take a big line to specify all header parameters, the parser will instead fill in the header
    m_fileHeader      = header;
    m_fileHeader.uuid = databaseEntry.uuid;

    std::ofstream file(m_path, std::ios::binary);

    //==================================
    // save to file and delete the object
    if(file.is_open())
    {
        file.write(reinterpret_cast<char*>(&m_fileHeader), sizeof(m_fileHeader));

        // Write texutre data
        size_t pixelSize = m_fileHeader.widht * m_fileHeader.height * m_fileHeader.channels;
        file.write(reinterpret_cast<const char*>(pixels), pixelSize);

        file.close();
    }
}

bool     VTexture::Save(std::filesystem::path& path) {}
bool     VTexture::Load() {}
std::any VTexture::LoadData() {}
}  // namespace ApplicationCore