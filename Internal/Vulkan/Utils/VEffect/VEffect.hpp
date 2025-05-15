//
// Created by wpsimon09 on 23/04/25.
//

#ifndef VEFFECT_HPP
#define VEFFECT_HPP
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
struct ReflectionData;
}
namespace VulkanCore {
class VDescriptorLayoutCache;
}
inline int EffectIndexCounter = 0;

namespace VulkanUtils {
class VShaderResourceGroup;


class VEffect
{
  public:
    explicit VEffect(const VulkanCore::VDevice&                          device,
                     const std::string&                                  name,
                     VulkanCore::VDescriptorLayoutCache&                 descriptoSetLayoutCache,
                     std::shared_ptr<VulkanUtils::VShaderResourceGroup>& descriptorSet);

  public:
    std::string&                  GetName();
    DescriptorSetTemplateVariant& GetResrouceGroupStructVariant();
    virtual void                  BuildEffect()                                    = 0;
    virtual vk::PipelineLayout    GetPipelineLayout()                              = 0;
    virtual void                  BindPipeline(const vk::CommandBuffer& cmdBuffer) = 0;
    virtual void                  Destroy()                                        = 0;
    vk::DescriptorUpdateTemplate& GetUpdateTemplate();
    unsigned short                EvaluateRenderingOrder();
    int&                          GetID();
    EDescriptorLayoutStruct       GetLayoutStructType();
    void                          CreateLayouts(const VulkanCore::ReflectionData& reflectionData);
    /**
     * Informs how many writes will be needed for each element in the descriptor sets
     * @param buffers number of buffers for writes
     * @param images number of images for writes
     * @param accels number of acceleration structures for writes
     */
    void                          SetNumWrites(uint32_t buffers = 1, uint32_t images = 1 , uint32_t accels = 1);

    //=====================================
    // DESCRIPTOR WRITES
    //=====================================
    void WriteBuffer(uint32_t frame, uint32_t set, uint32_t binding, vk::DescriptorBufferInfo bufferInfo);
    void WriteImage(uint32_t frame, uint32_t set, uint32_t binding, vk::DescriptorImageInfo imageInfo);
    void WriteAccelerationStrucutre(uint32_t frame, uint32_t set, uint32_t binding, vk::AccelerationStructureKHR& asInfo);
    void ApplyWrites(uint32_t frame);

    virtual void BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) = 0;

    //======================================
    // COPY OF RESOURCES
    //======================================
    std::vector<vk::DescriptorBufferInfo> m_bufferInfos;
    std::vector<vk::DescriptorImageInfo> m_imageInfos;
    std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> m_asInfos;

  protected:
    const VulkanCore::VDevice&                         m_device;
    std::string                                        m_name;
    std::shared_ptr<VulkanUtils::VShaderResourceGroup> m_resourceGroup;
    int                                                m_ID;

    //========================================================================
    // Effect descriptor data
    //========================================================================

    /**
     * There can be multiple descriptor sets per pipeline therefore this is an array
     */
    std::vector<VulkanStructs::VDescriptorSet> m_descriptorSets;
    std::vector<vk::DescriptorSetLayout>       m_descriptorSetLayouts;
    const VulkanCore::ReflectionData*          m_reflectionData;
    VulkanCore::VDescriptorLayoutCache&        m_descriptorSetLayoutCache;
};

}  // namespace VulkanUtils

#endif  //VEFFECT_HPP
