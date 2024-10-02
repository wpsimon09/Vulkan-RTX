//
// Created by wpsimon09 on 29/09/24.
//

#ifndef VSWAPCHAIN_HPP
#define VSWAPCHAIN_HPP

#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore{
    class VulkanInstance;
    class VDevice;

    class VSwapChain:public VObject {
        public:
            VSwapChain(const VulkanCore::VDevice& device,const VulkanCore::VulkanInstance& instance);
            ~VSwapChain() = default;
            void Destroy() override;

            const vk::SurfaceFormatKHR& GetSurfaceFormatKHR() { return m_format; };
            const vk::Extent2D& GetExtent() { return m_extent; };
            const vk::PresentModeKHR& GetPresentMode() { return m_presentMode; };
        private:
            vk::SurfaceFormatKHR m_format;
            vk::Extent2D m_extent;
            vk::PresentModeKHR m_presentMode;

            vk::SwapchainKHR m_swapChain;

            std::vector<vk::Image> m_images;
            std::vector<vk::ImageView> m_imageViews;

            const VulkanCore::VDevice& m_device;
            const VulkanCore::VulkanInstance& m_instance;
        private:
            void ChooseExtent();
            void ChooseFormat();
            void ChoosePresentMode();

            void CreateSwapChain();

            void RetrieveSwapChainImagesAndImageViews();
    };

}


#endif //VSWAPCHAIN_HPP
