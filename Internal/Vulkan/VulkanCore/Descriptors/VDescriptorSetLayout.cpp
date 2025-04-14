//
// Created by wpsimon09 on 29/10/24.
//

#include "VDescriptorSetLayout.hpp"

#include <functional>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VDescriptorSetLayout::VDescriptorSetLayout(const VulkanCore::VDevice&                                   device,
                                           std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : m_device(device)
    , m_descriptorSetLayoutBindings(std::move(bindings))
    , VObject()
{

    Utils::Logger::LogInfoVerboseOnly("Creating descriptor set layout");
    std::vector<vk::DescriptorSetLayoutBinding> setBindings;
    for(auto& binding : m_descriptorSetLayoutBindings)
    {
        setBindings.push_back(binding.second);
    }

    vk::DescriptorSetLayoutCreateInfo info{};
    info.bindingCount = static_cast<uint32_t>(setBindings.size());
    info.pBindings    = setBindings.data();
    info.flags        = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;

    m_descriptorSetLayout = m_device.GetDevice().createDescriptorSetLayout(info);
    assert(m_descriptorSetLayout && "Failed to create descriptor set layout");

    Utils::Logger::LogSuccess("Descriptor set layout created");
}

VDescriptorSetLayout::VDescriptorSetLayout(const VulkanCore::VDevice& device, const VulkanUtils::DescriptorSetTemplateVariant& dstSetTemplate)
    : m_device(device)
    , m_descriptorSetTemplateStruct(dstSetTemplate)
{
    std::visit(
        [this, &device](auto& templateStruct) {
            using t = std::decay_t<decltype(templateStruct)>;

            //if descriptors change this is where i have to update them
            if constexpr(std::is_same_v<t, VulkanUtils::EmtpyDescriptorSet>)
            {
                auto emptyDescriptorLayout    = VulkanCore::VDescriptorSetLayout::Builder(device);
                m_descriptorSetLayoutBindings = std::move(emptyDescriptorLayout.m_descriptorBindings);
            }

            if constexpr(std::is_same_v<t, VulkanUtils::BasicDescriptorSet>)
            {
                auto BasicDescriptorSetLayout =
                    VulkanCore::VDescriptorSetLayout::Builder(device)
                        // Global data (camera uniform buffer)
                        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        // Per object data (mesh uniform buffer)
                        .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        //extra data
                        .AddBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1);


                m_descriptorSetLayoutBindings = std::move(BasicDescriptorSetLayout.m_descriptorBindings);
            }
            else if constexpr(std::is_same_v<t, VulkanUtils::UnlitSingleTexture>)
            {
                auto UnlitSingleTextureLayout =
                    VulkanCore::VDescriptorSetLayout::Builder(device)
                        // Global data (camera uniform buffer)
                        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        // Per object data (mesh uniform buffer)
                        .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        // Extra data
                        .AddBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        // Texture (albedo)
                        .AddBinding(3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1);

                m_descriptorSetLayoutBindings = std::move(UnlitSingleTextureLayout.m_descriptorBindings);
            }
            else if constexpr(std::is_same_v<t, VulkanUtils::ForwardShadingDstSet>)
            {
                auto ForwardShadingDstSetLayout =
                    VulkanCore::VDescriptorSetLayout::Builder(device)
                        // Global data (camera uniform buffer)
                        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        // Per object data (mesh uniform buffer)
                        .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
                        // light data
                        .AddBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)

                        //Extre buffer
                        .AddBinding(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
                        // Material features
                        .AddBinding(4, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
                        // PBR not texture data
                        .AddBinding(5, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
                        // Albdeo texture
                        .AddBinding(6, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // Normal (Ambient, Roughness, Metallic)
                        .AddBinding(7, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // Normal texture
                        .AddBinding(8, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // Emmisive
                        .AddBinding(9, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // LTC (Linearly Transformed Cosines) lookup table
                        .AddBinding(10, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // LTC inverse lookup table
                        .AddBinding(11, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // irradiance map
                        .AddBinding(12, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // prefilter map
                        .AddBinding(13, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
                        // brdf map
                        .AddBinding(14, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1);

                m_descriptorSetLayoutBindings = std::move(ForwardShadingDstSetLayout.m_descriptorBindings);
            }
            else
            {
                // Handle unexpected type (optional)
                static_assert("Unsupported descriptor set type");
            }
        },
        dstSetTemplate);

    Utils::Logger::LogInfoVerboseOnly("Creating descriptor set layout");
    std::vector<vk::DescriptorSetLayoutBinding> setBindings;
    for(auto& binding : m_descriptorSetLayoutBindings)
    {
        setBindings.push_back(binding.second);
    }

    vk::DescriptorSetLayoutCreateInfo info{};
    info.bindingCount = static_cast<uint32_t>(setBindings.size());
    info.pBindings    = setBindings.data();
    info.flags        = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;

    m_descriptorSetLayout = m_device.GetDevice().createDescriptorSetLayout(info);
    assert(m_descriptorSetLayout && "Failed to create descriptor set layout");

    Utils::Logger::LogSuccess("Descriptor set layout created");
}

void VDescriptorSetLayout::Destroy()
{
    m_device.GetDevice().destroyDescriptorSetLayout(m_descriptorSetLayout);
}

VDescriptorSetLayout::Builder::Builder(const VulkanCore::VDevice& device)
    : m_device(device)
{
}


VDescriptorSetLayout::Builder& VDescriptorSetLayout::Builder::AddBinding(uint32_t             binding,
                                                                         vk::DescriptorType   type,
                                                                         vk::ShaderStageFlags stage,
                                                                         uint32_t             descriptorCount)
{

    assert(m_descriptorBindings.count(binding) == 0 && "Binding already exists");

    vk::DescriptorSetLayoutBinding bindingInfo{};
    bindingInfo.binding           = binding;
    bindingInfo.descriptorType    = type;
    bindingInfo.descriptorCount   = descriptorCount;
    bindingInfo.stageFlags        = stage;
    m_descriptorBindings[binding] = bindingInfo;

    return *this;
}

std::unique_ptr<VulkanCore::VDescriptorSetLayout> VDescriptorSetLayout::Builder::Build()
{
    return std::make_unique<VDescriptorSetLayout>(m_device, m_descriptorBindings);
}

}  // namespace VulkanCore