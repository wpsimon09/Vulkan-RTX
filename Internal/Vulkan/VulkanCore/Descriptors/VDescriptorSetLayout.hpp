//
// Created by wpsimon09 on 29/10/24.
//

#ifndef VDESCRIPTORSETLAYOUT_HPP
#define VDESCRIPTORSETLAYOUT_HPP
#include <memory>
#include <unordered_map>

#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>

#include "Vulkan/Utils/VResrouceGroup/VDescriptorSetStructs.hpp"


namespace VulkanUtils {

class VResourceGroupManager;
}

namespace VulkanCore {
class VDevice;
class VDescriptorSet;

/**
     * Class that represents descriptor set layout bindings it will contain binding for each descriptor
     * exam
     * 0, UBO
     * 1, UBO
     * 2, IMAGE
     * 3, UBO []
     */
class VDescriptorSetLayout : VObject
{
  public:
    class Builder
    {
      public:
        explicit Builder(const VulkanCore::VDevice& device);

        Builder& AddBinding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stage, uint32_t descriptorCount);

        std::unique_ptr<VulkanCore::VDescriptorSetLayout> Build();

        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_descriptorBindings;

      private:
        const VulkanCore::VDevice& m_device;
    };

  public:
    explicit VDescriptorSetLayout(const VulkanCore::VDevice&                                   device,
                                  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
    // generates layout basesd passed descriptor template struct
    explicit VDescriptorSetLayout(const VulkanCore::VDevice& device, const VulkanUtils::DescriptorSetTemplateVariant& dstSetTemplate);

    const vk::DescriptorSetLayout&             GetLayout() const { return m_descriptorSetLayout; }
    VulkanUtils::DescriptorSetTemplateVariant& GetStructure() { return m_descriptorSetTemplateStruct.value(); };

    auto& GetBindings() { return m_descriptorSetLayoutBindings; }

    void Destroy() override;

  private:
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_descriptorSetLayoutBindings;
    const VDevice&                                               m_device;
    vk::DescriptorSetLayout                                      m_descriptorSetLayout;
    std::optional<VulkanUtils::DescriptorSetTemplateVariant>     m_descriptorSetTemplateStruct;

    friend class VulkanCore::VDescriptorSet;
    friend class VulkanUtils::VResourceGroupManager;
};


}  // namespace VulkanCore

#endif  //VDESCRIPTORSETLAYOUT_HPP
