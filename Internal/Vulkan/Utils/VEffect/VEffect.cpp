//
// Created by wpsimon09 on 23/04/25.
//

#include "VEffect.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VResrouceGroup/VResrouceGroup.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace VulkanUtils {
VEffect::VEffect(const VulkanCore::VDevice&                          device,
                 const std::string&                                  name,
                 VulkanCore::VDescriptorLayoutCache&           descriptoSetLayoutCache,
                 std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet)
    : m_device(device)
    , m_name(name)
    , m_resourceGroup(descriptorSet)
    , m_descriptorSetLayoutCache(descriptoSetLayoutCache)
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
vk::DescriptorUpdateTemplate& VEffect::GetUpdateTemplate()
{
    return m_resourceGroup->GetUpdateTemplate();
}
unsigned short VEffect::EvaluateRenderingOrder()
{
    return 0;
}
int& VEffect::GetID()
{
    return m_ID;
}
EDescriptorLayoutStruct VEffect::GetLayoutStructType()
{
    return m_resourceGroup->GetResourceGroupStrucutureType();
}
void VEffect::CreateLayouts(const VulkanCore::ReflectionData& reflectionData)
{
    m_reflectionData = &reflectionData;

    m_descriptorSets.reserve(reflectionData.descriptorSets.size());

    for(auto& set : reflectionData.descriptorSets)
    {
        VulkanStructs::VDescriptorSet newSet;
        newSet.layouts = m_descriptorSetLayoutCache.CreateDescriptorSetLayout(&set.second.createInfo);

        // copy the layout for pieline creation
        m_descriptorSetLayouts.push_back(newSet.layouts);
        for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DescriptorsetCreate
            // create the sets from the pool
        }

        m_descriptorSets.push_back(newSet);
    }

    Utils::Logger::LogSuccess("Descriptor set layout created successfully");
}


}  // namespace VulkanUtils