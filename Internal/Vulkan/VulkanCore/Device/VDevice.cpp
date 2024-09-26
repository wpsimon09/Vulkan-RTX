//
// Created by wpsimon09 on 25/09/24.
//

#include "VDevice.hpp"

#include "Includes/Logger/Logger.hpp"

VulkacCore::VDevice::VDevice(vk::Instance &instance):m_insatnce(instance) {

}

vk::PhysicalDevice VulkacCore::VDevice::PickPhysicalDevice() {
    auto availablePhysicalDevices = m_insatnce.enumeratePhysicalDevices();
    for(auto  &physicalDevice: availablePhysicalDevices) {
        Utils::Logger::LogInfo("Found physical device: " + std::string(physicalDevice.getProperties().deviceName));
    }
}
