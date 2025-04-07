//
// Created by wpsimon09 on 01/04/25.
//

#ifndef VENVLIGHTGENERATOR_HPP
#define VENVLIGHTGENERATOR_HPP
#include <memory>
#include <unordered_map>
#include <glm/ext/matrix_transform.hpp>

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
        VEnvLightGenerator(const VulkanCore::VDevice& device, VulkanUtils::VPushDescriptorManager& pushDescriptorManager);

        const VulkanCore::VImage2&                  GetBRDFLut();
        VulkanCore::VImage2*                        GetBRDFLutRaw();

        const VulkanCore::VImage2&                  GetCubeMap();
        VulkanCore::VImage2*                        GetCubeMapRaw();

        const VulkanCore::VImage2&                  GetIrradianceMap();
        VulkanCore::VImage2*                        GetIrradianceMapRaw();

        VulkanCore::VImage2*                        GetPrefilterMapRaw();

        void                                        Generate(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);

        void                                        HDRToCubeMap(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);
        void                                        CubeMapToIrradiance(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);
        void                                        CubeMapToPrefilter(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);

        void                                        Destroy();

    private:
        void                                        GenerateBRDFLut();

    private:
        std::unique_ptr<VulkanCore::VImage2> m_brdfLut;

        //HDR
        std::unordered_map<int, std::unique_ptr<VulkanCore::VImage2>> m_irradianceMaps;
        std::unordered_map<int, std::unique_ptr<VulkanCore::VImage2>> m_prefilterMaps;
        std::unordered_map<int, std::unique_ptr<VulkanCore::VImage2>> m_hdrCubeMaps;
        int m_currentHDR;


        const VulkanCore::VDevice& m_device;

        VulkanUtils::VPushDescriptorManager& m_pushDescriptorManager;

        std::unique_ptr<VulkanCore::VCommandBuffer> m_graphicsCmdBuffer;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCmdBuffer;

        std::unique_ptr<VulkanCore::VCommandPool> m_graphicsCmdPool;
        std::unique_ptr<VulkanCore::VCommandPool> m_transferCmdPool;

        std::unique_ptr<ApplicationCore::StaticMesh> m_cube;

        std::vector<glm::mat4> m_camptureViews;


    };
}



#endif //VENVLIGHTGENERATOR_HPP
