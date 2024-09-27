//
// Created by wpsimon09 on 27/09/24.
//

#include "VGeneralUtils.hpp"
#include "Includes/Logger/Logger.hpp"


uint32_t VulkanUtils::FindQueueFamily(const std::vector<vk::QueueFamilyProperties> &queueFamilyProperties,
    vk::QueueFlagBits queueType) {
    //select just the queue fmily index that supports graphics operations
    std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty = std::find_if(
        queueFamilyProperties.begin(),
        queueFamilyProperties.end(),
        [queueType]( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & queueType; } );

    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    auto queueFamilyIndex =  static_cast<uint32_t> (std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    Utils::Logger::LogInfoVerboseOnly("Found graphics queue family at index: " + std::to_string(queueFamilyIndex));
    return queueFamilyIndex;
}
