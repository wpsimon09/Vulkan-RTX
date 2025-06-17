//
// Created by wpsimon09 on 08/02/25.
//
#include "LookUpTables.hpp"
#include "LTC_Values.h"
#include "ModelExportImportUtils/ModelManagmentUtils.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"

void MathUtils::InitLookUpTables(const VulkanCore::VDevice& device)
{
    auto& transferOpsManager = device.GetTransferOpsManager();

    LTC_ImageData        = {LTC, 64, 64, 4, MathUtils::LTC_String};
    LTC_ImageData.format = vk::Format::eR32G32B32A32Sfloat;

    LTCInverse_ImageData        = {LTC_Inverse, 64, 64, 4, MathUtils::LTC_InverseString};
    LTCInverse_ImageData.format = vk::Format::eR32G32B32A32Sfloat;

    //=======================================
    // LTC TEXTURES
    //=======================================
    auto ltcTexture = std::make_shared<VulkanCore::VImage2>(device, MathUtils::LTC_ImageData);
    transferOpsManager.DestroyBuffer(ltcTexture->GetImageStagingvBuffer(), true);
    LookUpTables.LTC = std::make_shared<ApplicationCore::VTextureAsset>(device, std::move(ltcTexture));

    ltcTexture = std::make_shared<VulkanCore::VImage2>(device, MathUtils::LTCInverse_ImageData);
    transferOpsManager.DestroyBuffer(ltcTexture->GetImageStagingvBuffer(), true);
    LookUpTables.LTCInverse = std::make_shared<ApplicationCore::VTextureAsset>(device, std::move(ltcTexture));

    //=======================================
    // BLUE NOISE TEXTURE ARRAY
    //=======================================
    auto blueNoiseTextureData = ApplicationCore::LoadTextureArray("Resources/BlueNoise/256");
    ltcTexture = std::make_shared<VulkanCore::VImage2>(device, blueNoiseTextureData );
    transferOpsManager.DestroyBuffer(ltcTexture->GetImageStagingvBuffer(), true);
    LookUpTables.BlueNoise64 = std::make_shared<ApplicationCore::VTextureAsset>(device, std::move(ltcTexture));
}
