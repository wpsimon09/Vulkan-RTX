//
// Created by wpsimon09 on 29/09/24.
//

#ifndef VSWAPCHAIN_HPP
#define VSWAPCHAIN_HPP

#include <vulkan/vulkan.hpp>

namespace VulkanCore{
    class VulkanInstance;
    class VDevice;

    class VSwapChain {
        public:
            VSwapChain(const VulkanCore::VDevice& device,const VulkanCore::VulkanInstance& instance);

            const vk::Format& GetImageFormat() { return m_format; };
            const vk::Extent2D& GetExtent() { return m_extent; };
            const vk::PresentModeKHR& GetPresentMode() { return m_presentMode; };
        private:
            vk::Format m_format;
            vk::Extent2D m_extent;
            vk::PresentModeKHR m_presentMode;
        private:


            void ChooseExtent();
            void ChooseFormat();
            void ChoosePresentMode();
    };

}


#endif //VSWAPCHAIN_HPP
