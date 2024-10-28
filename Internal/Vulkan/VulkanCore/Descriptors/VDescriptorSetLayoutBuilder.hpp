//
// Created by wpsimon09 on 28/10/24.
//

#ifndef VDESCRIPTORSETLAYOUTBUILDER_HPP
#define VDESCRIPTORSETLAYOUTBUILDER_HPP
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore {
class VDevice;
class VDescriptorSetLayoutBuilder:public VObject {
    public:
    VDescriptorSetLayoutBuilder(const VulkanCore::VDevice& device);



    private:
        std::pmr::unordered_map<uint32_t, >
        const VulkanCore::VDevice& device;
};

} // VulkanCore

#endif //VDESCRIPTORSETLAYOUTBUILDER_HPP
