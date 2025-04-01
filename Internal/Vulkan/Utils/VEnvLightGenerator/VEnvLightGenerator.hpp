//
// Created by wpsimon09 on 01/04/25.
//

#ifndef VENVLIGHTGENERATOR_HPP
#define VENVLIGHTGENERATOR_HPP
#include <memory>

#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanCore
{
    class VTimelineSemaphore;
}

namespace VulkanUtils
{
    class VTransferOperationsManager;
}

namespace VulkanCore
{
    class VImage2;
    class VDevice;
}

/**
 * This class intends to be quite large and its main purpose is to genera images for IBL, irradiance map, radiance map and HDR cube map
 */
namespace VulkanUtils
{
    class VEnvLightGenerator {
    public:
        VEnvLightGenerator(const VulkanCore::VDevice& device);

        void                                        Generate(VulkanCore::VImage2& envMap);
    private:
        void                                        GenerateBRDFLut();

    private:
        std::unique_ptr<VulkanCore::VImage2> m_brdfLut;
        const VulkanCore::VDevice& m_device;
        VulkanCore::VTimelineSemaphore m_envMapGenerationSemphore;
        VulkanUtils::VTransferOperationsManager m_envGenerationTransferOpsManager;


    };
}



#endif //VENVLIGHTGENERATOR_HPP
