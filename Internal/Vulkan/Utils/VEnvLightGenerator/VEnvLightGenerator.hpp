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
        void                                        Generate(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);

        void                                        HDRToCubeMap(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);
        void                                        CubeMapToIrradiance(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore);
        void                                        CubeMapToPrefilter(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                            VulkanCore::VTimelineSemaphore& renderingSemaphore)

        void                                        Destroy();
    private:
        void                                        GenerateBRDFLut();

    private:
        std::unique_ptr<VulkanCore::VImage2> m_brdfLut;

        //HDR
        std::unordered_map<std::shared_ptr<VulkanCore::VImage2>, std::unique_ptr<VulkanCore::VImage2>> m_irradianceMaps;
        std::unordered_map<std::shared_ptr<VulkanCore::VImage2>, std::unique_ptr<VulkanCore::VImage2>> m_prefilterMaps;
        std::unordered_map<std::shared_ptr<VulkanCore::VImage2>, std::unique_ptr<VulkanCore::VImage2>> m_hdrCubeMaps;


        const VulkanCore::VDevice& m_device;

        VulkanUtils::VPushDescriptorManager& m_pushDescriptorManager;

        std::unique_ptr<VulkanCore::VCommandBuffer> m_graphicsCmdBuffer;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCmdBuffer;

        std::unique_ptr<VulkanCore::VCommandPool> m_graphicsCmdPool;
        std::unique_ptr<VulkanCore::VCommandPool> m_transferCmdPool;

        VulkanStructs::MeshData m_cube;

        std::vector<glm::mat4> matrices = {
            // POSITIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

    };
}



#endif //VENVLIGHTGENERATOR_HPP
