//
// Created by wpsimon09 on 29/09/24.
//

#include "VSwapChain.hpp"

#include "Includes/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Includes/WindowManager/WindowManager.hpp"

VulkanCore::VSwapChain::VSwapChain(const VulkanCore::VDevice &device, const VulkanCore::VulkanInstance &instance): m_device(device), m_instance(instance) {
    ChooseExtent();
    //ChooseFormat();
    //ChoosePresentMode();
}

void VulkanCore::VSwapChain::ChooseExtent() {
    Utils::Logger::LogInfo("Choosing right extend...");
    auto capabilities = m_device.GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_instance.GetSurface());
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        m_extent = capabilities.currentExtent;
        Utils::Logger::LogInfoVerboseOnly("Going to use default swap chain extend");
        return;
    }

    vk::Extent2D acctualExtend ={
        static_cast<uint32_t>(WindowManager::GetWindowWidth()),
        static_cast<uint32_t>(WindowManager::GetWindowHeight()),
    };

    acctualExtend.width = std::clamp(acctualExtend.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
    acctualExtend.height = std::clamp(acctualExtend.height, capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);

    m_extent = acctualExtend;
    Utils::Logger::LogInfoVerboseOnly("Current extend of the window is going ot be set \t Width:: " + std::to_string(acctualExtend.width) + "\n"  + "\tHeight:: " + std::to_string(acctualExtend.height));
}