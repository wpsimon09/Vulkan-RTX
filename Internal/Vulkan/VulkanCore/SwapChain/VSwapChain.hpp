//
// Created by wpsimon09 on 29/09/24.
//

#ifndef VSWAPCHAIN_HPP
#define VSWAPCHAIN_HPP

#include <memory>
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore{
    class VImage;
    class VRenderPass;
    class VFrameBuffer;
    class VulkanInstance;
    class VDevice;

    class VSwapChain:public VObject {
        public:
            VSwapChain(const VulkanCore::VDevice& device,const VulkanCore::VulkanInstance& instance);

            void Destroy() override;
            void CreateSwapChainFrameBuffers(const VulkanCore::VRenderPass& renderPass);
            void RecreateSwapChain(const VulkanCore::VRenderPass& renderPass);

            ~VSwapChain() = default;

            const vk::SwapchainKHR& GetSwapChain() const { return m_swapChain; };
            const vk::SurfaceFormatKHR& GetSurfaceFormatKHR() const { return m_format; };
            const vk::Extent2D& GetExtent() const { return m_extent; };
            const vk::PresentModeKHR& GetPresentMode() const { return m_presentMode; };
            const std::vector<std::reference_wrapper<const VulkanCore::VImage>> GetImages() const;
            const std::vector<std::reference_wrapper<const VulkanCore::VFrameBuffer>> GetSwapChainFrameBuffers() const;
        private:
            vk::SurfaceFormatKHR m_format;
            vk::Extent2D m_extent;
            vk::PresentModeKHR m_presentMode;

            vk::SwapchainKHR m_swapChain;

            std::vector<std::unique_ptr<VulkanCore::VImage>> m_images;
            std::vector<std::unique_ptr<VulkanCore::VFrameBuffer>> m_swapChainFrameBuffers;

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
