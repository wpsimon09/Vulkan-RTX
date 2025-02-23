#include "VTextureAsset.hpp"
#include "VAsset.hpp"

ApplicationCore::VTextureAsset::VTextureAsset(const VulkanCore::VDevice &device, std::filesystem::path texturePath) : VAsset<VulkanCore::VImage>(device)
{
    m_deviceHandle = std::make_shared<VulkanCore::VImage>(m_device);
    m_width = 1;
    m_height = 1;
    m_isLoaded = false;
    m_mipLevels = 1;
    m_savable = false;
    m_name = texturePath.filename().string();
    

    Load();
}

void ApplicationCore::VTextureAsset::Sync()
{

}

void ApplicationCore::VTextureAsset::LoadInternal()
{
    
}

void ApplicationCore::VTextureAsset::LoadInternalFromBuffer()
{
}
