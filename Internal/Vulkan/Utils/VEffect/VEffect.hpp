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
class VShaderResrouceGroup;


class VEffect
{
  public:
    explicit VEffect(const VulkanCore::VDevice&                          device,
                     const std::string&                                  name,
                     VulkanCore::VDescriptorLayoutCache&                 descriptoSetLayoutCache,
                     std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet);

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

  protected:
    const VulkanCore::VDevice&                         m_device;
    std::string                                        m_name;
    std::shared_ptr<VulkanUtils::VShaderResrouceGroup> m_resourceGroup;
    int                                                m_ID;

    //========================================================================
    // Effect descriptor data
    //========================================================================
    std::vector<VulkanStructs::VDescriptorSet> m_descriptorSets;
    std::vector<vk::DescriptorSetLayout>       m_descriptorSetLayouts;
    const VulkanCore::ReflectionData*          m_reflectionData;
    VulkanCore::VDescriptorLayoutCache&        m_descriptorSetLayoutCache;

};

}  // namespace VulkanUtils

#endif  //VEFFECT_HPP
