//
// Created by wpsimon09 on 25/09/24.
//
#pragma once
#ifndef VDEVICE_HPP
#define VDEVICE_HPP

#include <memory>
#include<vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"

namespace VulkanCore
{
    class VulkanInstance;
}

namespace VulkanCore
{
    struct VQueueFamilyIndices
    {
        std::optional<std::pair<QUEUE_FAMILY_INDEX_TYPE, uint32_t>> graphicsFamily;
        std::optional<std::pair<QUEUE_FAMILY_INDEX_TYPE, uint32_t>> presentFamily;
        std::optional<std::pair<QUEUE_FAMILY_INDEX_TYPE, uint32_t>> computeFamily;

        bool isComplete() const {
            return ( graphicsFamily.has_value() && presentFamily.has_value() ) ||computeFamily.has_value();
        };
    };


    VQueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& physicalDevice, const VulkanCore::VulkanInstance& instance);


    class VDevice: public VObject
    {
    public:
        VDevice(const VulkanCore::VulkanInstance& instance);

        const vk::PhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; } ;
        const vk::Device& GetDevice() const {return m_device;};
        const VQueueFamilyIndices& GetQueueFamilyIndices() const {return m_queueFamilyIndices;};
        const uint32_t& GetConcreteQueueFamilyIndex(QUEUE_FAMILY_INDEX_TYPE queueFamilyType) const;
        const std::string GetQueueFamilyString(QUEUE_FAMILY_INDEX_TYPE queueFamilyType) const;
        virtual void Destroy() override;

    private:
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device; //logical device

        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_transferQueue;
        vk::Queue m_presentQueue;

        VQueueFamilyIndices m_queueFamilyIndices;

        const VulkanCore::VulkanInstance& m_instance;

    private:
        vk::PhysicalDevice PickPhysicalDevice();
        void CreateLogicalDevice();
    };
}

#endif