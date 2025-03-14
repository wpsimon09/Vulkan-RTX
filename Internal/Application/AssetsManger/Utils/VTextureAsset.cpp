#include "VTextureAsset.hpp"

#include "simdjson.h"
#include "VAsset.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Application/Utils/ModelExportImportUtils/ModelManagmentUtils.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

ApplicationCore::VTextureAsset::VTextureAsset(
    const VulkanCore::VDevice &device,
    VulkanUtils::VTransferOperationsManager& transferOpsManager,
    std::shared_ptr<VulkanCore::VImage2> defaultTexture,
    ETextureAssetType type,
    std::filesystem::path texturePath) : VAsset<VulkanCore::VImage2>(device), m_textureAssetType(type),
                                         m_originalPathToTexture(texturePath), m_transferOpsManager(transferOpsManager)
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

ApplicationCore::VTextureAsset::VTextureAsset(
    const VulkanCore::VDevice                   &device,
    VulkanUtils::VTransferOperationsManager&    transferOpsManager,
    std::shared_ptr<VulkanCore::VImage2>         defaultTexture,
    ETextureAssetType type, TextureBufferInfo&  bufferInfo): VAsset<VulkanCore::VImage2>(device),
                                                            m_textureAssetType(type), m_textureBufferInfo(bufferInfo),
                                                            m_transferOpsManager(transferOpsManager)
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

ApplicationCore::VTextureAsset::VTextureAsset(
    const VulkanCore::VDevice& device, VulkanUtils::VTransferOperationsManager& transferOpsManager,
    std::shared_ptr<VulkanCore::VImage2> texture): VAsset<VulkanCore::VImage2>(device),
                                                  m_textureAssetType(ETextureAssetType::Texture),
                                                  m_textureBufferInfo(nullptr), m_transferOpsManager(transferOpsManager)
{
    m_deviceHandle = texture;
    m_width = 1;
    m_height = 1;
    m_isLoaded = false;
    m_mipLevels = 1;
    m_savable = false;
    m_textureSource = EImageSource::Buffer;
    m_assetPath = texture->GetImageInfo().imagePath;
}

void ApplicationCore::VTextureAsset::Sync()
{
    if(m_isInSync)
        return;
    if (m_loadedImageData.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready ){
        return;
    }

    m_isInSync = true;
    auto imageData = m_loadedImageData.get();
    m_assetPath = imageData.fileName;
    m_deviceHandle = std::make_shared<VulkanCore::VImage2>(m_device,m_transferOpsManager.GetCommandBuffer(),imageData);
    m_transferOpsManager.DestroyBuffer(m_deviceHandle->GetImageStagingvBuffer(), true);
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

std::shared_ptr<VulkanCore::VImage2> ApplicationCore::VTextureAsset::GetHandle()
{
    return VAsset<VulkanCore::VImage2>::GetHandle();
}

VulkanCore::VImage2& ApplicationCore::VTextureAsset::GetHandleByRef()
{
    return VAsset<VulkanCore::VImage2>::GetHandleByRef();
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
