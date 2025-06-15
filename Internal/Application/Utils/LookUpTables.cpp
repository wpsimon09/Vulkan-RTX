//
// Created by wpsimon09 on 08/02/25.
//
#include "LookUpTables.hpp"
#include "LTC_Values.h"

void MathUtils::InitLookUpTables()
{
    LTC_ImageData        = {LTC, 64, 64, 4, MathUtils::LTC_String};
    LTC_ImageData.format = vk::Format::eR32G32B32A32Sfloat;

    LTCInverse_ImageData        = {LTC_Inverse, 64, 64, 4, MathUtils::LTC_InverseString};
    LTCInverse_ImageData.format = vk::Format::eR32G32B32A32Sfloat;

    BlueNoiseString = std::make_shared<ApplicationCore::VTextureAsset>()
}
