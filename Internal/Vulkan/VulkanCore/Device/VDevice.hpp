//
// Created by wpsimon09 on 25/09/24.
//
#pragma once
#ifndef VDEVICE_HPP
#define VDEVICE_HPP

#include <memory>
#include<vulkan/vulkan.hpp>


namespace VulkanCore
{
    struct VQueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;

        bool isComplete() const {return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();};
    };

    VQueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& physicalDevice);


    class VDevice
    {
    public:
        VDevice(const vk::Instance& instance);

        const vk::PhysicalDevice& GetPhysicalDevice() const;
        const vk::Device& GetDevice() {return m_device;};
        const VQueueFamilyIndices& GetQueueFamilyIndices() {return m_queueFamilyIndices;};

        ~VDevice();
    private:
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device; //logical device

        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_transferQueue;

        VQueueFamilyIndices m_queueFamilyIndices;

        const vk::Instance& m_instance;

    private:
        vk::PhysicalDevice PickPhysicalDevice();
        void CreateLogicalDevice();
    };
}

#endif