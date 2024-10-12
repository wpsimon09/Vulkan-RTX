//
// Created by wpsimon09 on 29/09/24.
//

#ifndef VSWAPCHAIN_HPP
#define VSWAPCHAIN_HPP

#include <memory>
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanUtils
{
    class VImage;
}

namespace VulkanCore{
    class VulkanInstance;
    class VDevice;

    class VSwapChain:public VObject {
        public:
            VSwapChain(const VulkanCore::VDevice& device,const VulkanCore::VulkanInstance& instance);
            ~VSwapChain() = default;
            void Destroy() override;

            const vk::SurfaceFormatKHR& GetSurfaceFormatKHR() const { return m_format; };
            const vk::Extent2D& GetExtent() const { return m_extent; };
            const vk::PresentModeKHR& GetPresentMode() const { return m_presentMode; };
        private:
            vk::SurfaceFormatKHR m_format;
            vk::Extent2D m_extent;
            vk::PresentModeKHR m_presentMode;

            vk::SwapchainKHR m_swapChain;

            std::vector<VulkanUtils::VImage> m_images;

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
