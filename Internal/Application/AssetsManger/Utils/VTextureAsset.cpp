#include "VTextureAsset.hpp"
#include "VAsset.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Application/Utils/ModelExportImportUtils/ModelManagmentUtils.hpp"

ApplicationCore::VTextureAsset::VTextureAsset(const VulkanCore::VDevice &device, std::shared_ptr<VulkanCore::VImage> defaultTexture, ETextureAssetType type,  std::filesystem::path texturePath) : VAsset<VulkanCore::VImage>(device), m_textureAssetType(type), m_originalPathToTexture(texturePath)
{
    m_deviceHandle = defaultTexture;
    m_width = 1;
    m_height = 1;
    m_isLoaded = false;
    m_mipLevels = 1;
    m_savable = m_textureAssetType != ETextureAssetType::EditorBillboard;
    m_textureSource = EImageSource::File;
    m_assetPath = texturePath;

    VTextureAsset::Load();
}

ApplicationCore::VTextureAsset::VTextureAsset(const VulkanCore::VDevice &device,std::shared_ptr<VulkanCore::VImage> defaultTexture, ETextureAssetType type, TextureBufferInfo &bufferInfo): VAsset<VulkanCore::VImage>(device), m_textureAssetType(type), m_textureBufferInfo(bufferInfo)
{
    m_deviceHandle = defaultTexture;
    m_width = 1;
    m_height = 1;
    m_isLoaded = false;
    m_mipLevels = 1;
    m_savable = m_textureAssetType != ETextureAssetType::EditorBillboard;
    m_textureSource = EImageSource::Buffer;
    m_assetPath = bufferInfo.textureID;

    VTextureAsset::Load();
    
}

void ApplicationCore::VTextureAsset::Sync()
{
    if(m_isInSync)
        return;

    //Destroy();
    m_isInSync = true;
    auto imageData = m_loadedImageData.get();
    m_assetPath = imageData.fileName;
    m_deviceHandle = std::make_shared<VulkanCore::VImage>(m_device,imageData);
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

std::shared_ptr<VulkanCore::VImage> ApplicationCore::VTextureAsset::GetHandle()
{
    if(m_isInSync)
        return VAsset<VulkanCore::VImage>::GetHandle();

    if (m_loadedImageData.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        Destroy();
        m_isInSync = true;
        auto retrievedData = m_loadedImageData.get();
        m_deviceHandle = std::make_shared<VulkanCore::VImage>(m_device, retrievedData);
    }
    return VAsset<VulkanCore::VImage>::GetHandle();
}

VulkanCore::VImage& ApplicationCore::VTextureAsset::GetHandleByRef()
{
    if(m_isInSync)
        return VAsset<VulkanCore::VImage>::GetHandleByRef();

    if (m_loadedImageData.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        m_isInSync = true;
        //Destroy();
        auto retrievedData = m_loadedImageData.get();
        m_deviceHandle = std::make_shared<VulkanCore::VImage>(m_device, retrievedData);
    }
    return VAsset<VulkanCore::VImage>::GetHandleByRef();
}

void ApplicationCore::VTextureAsset::LoadInternal()
{
        m_loadedImageData = std::async([this](){
        if(m_originalPathToTexture.has_value()){
            return ApplicationCore::LoadImage(this->m_originalPathToTexture.value(), m_savable);
        }
        throw std::logic_error("Expected struct with information about buffer ");

    });
}

void ApplicationCore::VTextureAsset::LoadInternalFromBuffer()
{
        m_loadedImageData = std::async([this](){
        if(m_textureBufferInfo.has_value()){
            return ApplicationCore::LoadImage(this->m_textureBufferInfo.value(), m_textureBufferInfo.value().textureID, m_savable);
        }
        throw std::logic_error("Expected struct with information about buffer ");
    });
}
