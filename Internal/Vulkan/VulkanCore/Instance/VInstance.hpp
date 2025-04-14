//
// Created by wpsimon09 on 22/09/24.
//
#pragma once

#ifndef VINSTANCE_HPP
#define VINSTANCE_HPP

#include <string>
#include <vulkan/vulkan.hpp>


class GLFWwindow;

namespace VulkanCore {
class VulkanInstance
{
  public:
    explicit VulkanInstance(std::string appName, GLFWwindow* window);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                           VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                           void* /*pUserData*/);

    const vk::Instance&   GetInstance() const { return m_instance; }
    const vk::SurfaceKHR& GetSurface() const { return m_surface; }

    const std::vector<const char*>& GetInstanceExtensions() const { return m_instanceExtensions; };

    ~VulkanInstance();

  private:
    vk::SurfaceKHR             m_surface;
    vk::Instance               m_instance;
    vk::DebugUtilsMessengerEXT m_debugMessenger;

    std::vector<const char*> m_instanceExtensions;

  private:
    void CreateInstance(std::string appName);
    void CreateDebugUtilsMessenger();
    void CreateSurface(GLFWwindow* window);
};
}  // namespace VulkanCore

#endif  //VINSTANCE_HPP
