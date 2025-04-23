//
// Created by wpsimon09 on 23/04/25.
//

#include "VEffect.hpp"

#include "Vulkan/Utils/VResrouceGroup/VResrouceGroup.hpp"

namespace VulkanUtils {
VEffect::VEffect(const VulkanCore::VDevice& device, const std::string& name, std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet)
    : m_device(device)
    , m_name(name)
    , m_resourceGroup(descriptorSet)
{
    m_ID = EffectIndexCounter++;
}
std::string& VEffect::GetName()
{
    return m_name;
}
DescriptorSetTemplateVariant& VEffect::GetResrouceGroupStructVariant()
{
    return m_resourceGroup->GetResourceGroupStruct();
}
vk::DescriptorUpdateTemplate& VEffect::GetUpdateTemplate() { return m_resourceGroup->GetUpdateTemplate(); }
unsigned short                VEffect::EvaluateRenderingOrder() { return 0; }
int&                          VEffect::GetID() {return m_ID;}
EDescriptorLayoutStruct       VEffect::GetLayoutStructType() {}
}  // namespace VulkanUtils