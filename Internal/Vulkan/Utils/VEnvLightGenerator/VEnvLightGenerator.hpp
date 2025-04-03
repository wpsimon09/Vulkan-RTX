//
// Created by wpsimon09 on 01/04/25.
//

#ifndef VENVLIGHTGENERATOR_HPP
#define VENVLIGHTGENERATOR_HPP
#include <memory>
#include <unordered_map>

#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace ApplicationCore
{
    class StaticMesh;
}

namespace VulkanCore
{
    class VCommandPool;
}

namespace VulkanUtils
{
    class VPushDescriptorManager;
}

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
        VEnvLightGenerator(const VulkanCore::VDevice& device, VulkanCore::VTimelineSemaphore& renderingSemaphore, VulkanUtils::VPushDescriptorManager& pushDescriptorManager);

        const VulkanCore::VImage2&                  GetBRDFLut();
        void                                        Generate(VulkanCore::VImage2* envMap,
                                                            std::shared_ptr<ApplicationCore::StaticMesh> cubeMesh);

        void                                        Destroy();
    private:
        void                                        GenerateBRDFLut();

    private:
        std::unique_ptr<VulkanCore::VImage2> m_brdfLut;

        //HDR
        std::unordered_map<VulkanCore::VImage2*, std::unique_ptr<VulkanCore::VImage2>> m_irradianceMaps;
        std::unordered_map<VulkanCore::VImage2*, std::unique_ptr<VulkanCore::VImage2>> m_prefilterMaps;


        const VulkanCore::VDevice& m_device;

        VulkanCore::VTimelineSemaphore& m_renderingSemaphore;
        VulkanUtils::VPushDescriptorManager& m_pushDescriptorManager;


        std::unique_ptr<VulkanCore::VCommandBuffer> m_graphicsCmdBuffer;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCmdBuffer;

        std::unique_ptr<VulkanCore::VCommandPool> m_graphicsCmdPool;
        std::unique_ptr<VulkanCore::VCommandPool> m_transferCmdPool;

    };
}



#endif //VENVLIGHTGENERATOR_HPP
