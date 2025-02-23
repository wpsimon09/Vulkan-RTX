#include "VTextureAsset.hpp"
#include "VAsset.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Application/Utils/ModelExportImportUtils/ModelManagmentUtils.hpp"

ApplicationCore::VTextureAsset::VTextureAsset(const VulkanCore::VDevice &device, ETextureAssetType type,  std::filesystem::path texturePath) : VAsset<VulkanCore::VImage>(device), m_textureAssetType(type)
{
    m_deviceHandle = std::make_shared<VulkanCore::VImage>(m_device);
    m_width = 1;
    m_height = 1;
    m_isLoaded = false;
    m_mipLevels = 1;
    m_savable = m_textureAssetType == ETextureAssetType::EditorBillboard ? false : true;
    m_textureSource = EImageSource::File;
    m_originalPathToTexture.value() = texturePath;

    Load();
}

ApplicationCore::VTextureAsset::VTextureAsset(const VulkanCore::VDevice &device, ETextureAssetType type, TextureBufferInfo &bufferInfo): VAsset<VulkanCore::VImage>(device), m_textureAssetType(type)
{
    m_deviceHandle = std::make_shared<VulkanCore::VImage>(m_device);
    m_width = 1;
    m_height = 1;
    m_isLoaded = false;
    m_mipLevels = 1;
    m_savable = m_textureAssetType == ETextureAssetType::EditorBillboard ? false : true;
    m_textureSource = EImageSource::Buffer;
    m_textureBufferInfo.value() = bufferInfo;

    Load();
    
}

void ApplicationCore::VTextureAsset::Sync()
{
    if(m_isInSync || !m_loadedImageData.has_value())
        return;

    m_device.DeviceMutex.lock();
    if(m_futureDeviceHandle.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
        m_deviceHandle->FillWithImageData(m_loadedImageData.value());
        m_isInSync = true;
    }
}

void ApplicationCore::VTextureAsset::Destroy()
{
    if(m_deviceHandle){
        m_deviceHandle->Destroy();
    }
}

void ApplicationCore::VTextureAsset::Load()
{
    m_isInSync = false;
    if(m_textureSource == EImageSource::File){
        LoadInternal();
    }else{
        LoadInternalFromBuffer();
    }
}

void ApplicationCore::VTextureAsset::LoadInternal()
{
    m_futureDeviceHandle = std::async([this](){
        if(m_originalPathToTexture.has_value()){
            auto retrievedData = ApplicationCore::LoadImage(this->m_originalPathToTexture.value(), m_savable);
            auto loadedImage = std::make_shared<VulkanCore::VImage>(m_device);
            loadedImage->FillWithImageData<>(retrievedData, true, true);
            return std::move(loadedImage);
        }
        throw std::logic_error("Expected struct with information about buffer ");

    });
}

void ApplicationCore::VTextureAsset::LoadInternalFromBuffer()
{
    m_futureDeviceHandle = std::async([this](){
        if(m_textureBufferInfo.has_value()){
            auto retrievedData = ApplicationCore::LoadImage(this->m_textureBufferInfo.value(), m_textureBufferInfo.value().textureID, m_savable);
            auto loadedImage = std::make_shared<VulkanCore::VImage>(m_device);
            loadedImage->FillWithImageData<>(retrievedData, true, true);
            return std::move(loadedImage);
        }
        throw std::logic_error("Expected struct with information about buffer ");
    });
}
