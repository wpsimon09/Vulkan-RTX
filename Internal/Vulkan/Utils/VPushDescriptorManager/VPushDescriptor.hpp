//
// Created by wpsimon09 on 18/03/25.
//

#ifndef VPUSHDESCRIPTOR_HPP
#define VPUSHDESCRIPTOR_HPP
#include <memory>
#include <string>
#include <vector>

#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"

namespace VulkanCore
{
    class VDescriptorSetLayout;
    class VDevice;
}

namespace VulkanUtils {

template<typename T>
class VPushDescriptorSet
{
public:
    VPushDescriptorSet(const VulkanCore::VDevice &device,std::string& name, std::unique_ptr<VulkanCore::VDescriptorSetLayout> dstLayout,T& dstStruct)
        :m_device(device), m_name(name), m_dstLayout(std::move(dstLayout)), m_dstStruct(dstStruct)  {};

    T& GetDstStruct() {return m_dstStruct;};
    VulkanCore::VDescriptorSetLayout& GetLayout(){return *m_dstLayout; }
    std::string& GetName(){return m_name;};
    void AddUpdateEntry(uint32_t binding,size_t offset, size_t stride );


private:
    const VulkanCore::VDevice &m_device;
    T m_dstStruct;
    std::string m_name;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_dstLayout;
    std::vector<vk::DescriptorUpdateTemplateEntry> m_descriptorTemplateEntries;
};

//======================================================================================================
//======================================================================================================
// IMPLEMENTATION
//======================================================================================================
//======================================================================================================

template <typename T>
void VPushDescriptorSet<T>::AddUpdateEntry(uint32_t binding, size_t offset, size_t stride)
{

}


} // VulkanUtils

#endif //VPUSHDESCRIPTOR_HPP
