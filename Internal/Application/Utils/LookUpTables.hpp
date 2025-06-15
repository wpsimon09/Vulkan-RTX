//
// Created by wpsimon09 on 08/02/25.
//

#ifndef LOOKUPTABLES_HPP
#define LOOKUPTABLES_HPP
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace VulkanCore {
class VImage2;
}

namespace MathUtils {
// LTC1 is the inverse M
// LTC2 is for (GGX norm, fresnel, 0(unused), sphere for horizon-clipping)

inline constexpr std::string LTC_InverseString = "LTC_Inverse";
inline constexpr std::string LTC_String        = "LTC";
inline constexpr std::string BlueNoiseString   = "BlueNoise";

inline VulkanStructs::VImageData<float> LTC_ImageData;
inline VulkanStructs::VImageData<float> LTCInverse_ImageData;

class LookUpTalbes
{
  public:
    std::shared_ptr<ApplicationCore::VTextureAsset> LTC;
    std::shared_ptr<ApplicationCore::VTextureAsset> LTCInverse;
    /*
     *Texture array
     */
    std::shared_ptr<ApplicationCore::VTextureAsset> BlueNoise;

    void ClearLoopUpTables()
    {
        LTC->Destroy();
        LTCInverse->Destroy();
        BlueNoise->Destroy();
    }
};

inline LookUpTalbes LUT;

void InitLookUpTables();
}  // namespace MathUtils

#endif  //LOOKUPTABLES_HPP
