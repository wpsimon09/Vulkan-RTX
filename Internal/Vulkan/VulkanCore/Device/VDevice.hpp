//
// Created by wpsimon09 on 25/09/24.
//
#pragma once
#ifndef VDEVICE_HPP
#define VDEVICE_HPP

#include<vulkan/vulkan.hpp>


namespace VulkacCore
{
    class VDevice
    {
    public:
        VDevice(vk::Instance& instance);
    private:
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device; //logical device

        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_transferQueue;

        vk::Instance& m_insatnce;
    };
}

#endif