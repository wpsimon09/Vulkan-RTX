//
// Created by wpsimon09 on 01/04/25.
//

#ifndef VENVLIGHTGENERATOR_HPP
#define VENVLIGHTGENERATOR_HPP
#include "Vulkan/Utils/VPipelineBarriers.hpp"


#include <memory>
#include <unordered_map>
#include <glm/ext/matrix_transform.hpp>

#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanCore {
class VTimelineSemaphore2;
class VDescriptorLayoutCache;
}
namespace VulkanCore {
struct VImage2CreateInfo;
}

namespace ApplicationCore {
class StaticMesh;
}

namespace VulkanCore {
class VCommandPool;
}

namespace VulkanUtils {
class VResourceGroupManager;
}

namespace VulkanCore {
class VTimelineSemaphore;
}

namespace VulkanUtils {
class VTransferOperationsManager;
}

namespace VulkanCore {
class VImage2;
class VDevice;
}  // namespace VulkanCore

/**
 * This class intends to be quite large and its main purpose is to genera images for IBL, irradiance map, radiance map and HDR cube map
 */
namespace VulkanUtils {
class VEnvLightGenerator
{
  public:
    VEnvLightGenerator(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache);

    const VulkanCore::VImage2& GetBRDFLut();
    VulkanCore::VImage2*       GetBRDFLutRaw();

    const VulkanCore::VImage2& GetCubeMap();
    VulkanCore::VImage2*       GetCubeMapRaw();

    const VulkanCore::VImage2& GetIrradianceMap();
    VulkanCore::VImage2*       GetIrradianceMapRaw();

    VulkanCore::VImage2* GetDummyCubeMapRaw();

    VulkanCore::VImage2* GetPrefilterMapRaw();

    void Generate(uint32_t m_currentFrame, std::shared_ptr<VulkanCore::VImage2> envMap, VulkanCore::VTimelineSemaphore2& renderingSemaphore);

    void HDRToCubeMap(std::shared_ptr<VulkanCore::VImage2> envMap, VulkanCore::VTimelineSemaphore2& renderingSemaphore);
    void CubeMapToIrradiance(std::shared_ptr<VulkanCore::VImage2> envMap, VulkanCore::VTimelineSemaphore2& renderingSemaphore);
    void CubeMapToPrefilter(std::shared_ptr<VulkanCore::VImage2> envMap, VulkanCore::VTimelineSemaphore2& renderingSemaphore);

    void Destroy();

  private:
    void GenerateBRDFLut();
    void GenerateBRDFLutCompute();

  private:
    std::unique_ptr<VulkanCore::VImage2> m_brdfLut;

    //HDR
    std::unordered_map<int, std::unique_ptr<VulkanCore::VImage2>> m_irradianceMaps;
    std::unordered_map<int, std::unique_ptr<VulkanCore::VImage2>> m_prefilterMaps;
    std::unordered_map<int, std::unique_ptr<VulkanCore::VImage2>> m_hdrCubeMaps;
    std::unique_ptr<VulkanCore::VImage2>                          m_dummyCubeMap;
    int                                                           m_currentHDR;
    uint32_t                                                      m_currentFrame;

    const VulkanCore::VDevice&          m_device;
    VulkanCore::VDescriptorLayoutCache& m_descLayoutChache;

    VulkanCore::VCommandBuffer* m_graphicsCmdBuffer;
    std::unique_ptr<VulkanCore::VCommandBuffer> m_transferCmdBuffer;

    std::unique_ptr<VulkanCore::VCommandPool> m_graphicsCmdPool;
    std::unique_ptr<VulkanCore::VCommandPool> m_transferCmdPool;


    std::unique_ptr<ApplicationCore::StaticMesh> m_cube;

    std::vector<glm::mat4> m_camptureViews;

    std::unique_ptr<VulkanUtils::VRasterEffect> m_hdrToIrradianceEffect;
    std::unique_ptr<VulkanUtils::VRasterEffect> m_prefilterEffect;
    std::unique_ptr<VulkanUtils::VRasterEffect> m_hdrToCubeMapEffect;

    void RenderToCubeMap(const vk::CommandBuffer& cmdBuffer, vk::Viewport& viewport, vk::RenderingAttachmentInfo& attachment);

    void CopyResukt(const vk::CommandBuffer& cmdBuffer, const vk::Image& src, const vk::Image& dst, int w, int h, int m = 0, int f = 0);

    void CreateResources(const vk::CommandBuffer&              cmdBuffer,
                         std::unique_ptr<VulkanCore::VImage2>& cubeMap,
                         std::unique_ptr<VulkanCore::VImage2>& renderTarget,
                         VulkanCore::VImage2CreateInfo&        createInfo,
                         VulkanCore::VTimelineSemaphore&       semaphore);

    VulkanUtils::VBarrierPosition ColorAttachment_To_TransferSrc{vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                                 vk::AccessFlagBits2::eColorAttachmentWrite,
                                                                 vk::PipelineStageFlagBits2::eCopy,
                                                                 vk::AccessFlagBits2::eTransferRead};

    VulkanUtils::VBarrierPosition TransferSrc_To_ColorAttachment{vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
                                                                 vk::PipelineStageFlagBits2::eColorAttachmentOutput | vk::PipelineStageFlagBits2::eFragmentShader,
                                                                 vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eShaderSampledRead};

    VulkanUtils::VBarrierPosition TransferDst_To_ReadOnly = {vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
                                                             vk::PipelineStageFlagBits2::eFragmentShader,
                                                             vk::AccessFlagBits2::eShaderRead};
};
}  // namespace VulkanUtils


#endif  //VENVLIGHTGENERATOR_HPP
