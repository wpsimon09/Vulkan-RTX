//
// Created by wpsimon09 on 29/09/24.
//

#ifndef VSWAPCHAIN_HPP
#define VSWAPCHAIN_HPP

#include <memory>
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace Renderer
{
    class RenderTarget;
}

namespace VulkanCore
{
    class VImage;
    class VulkanInstance;
    class VDevice;

    class VSwapChain : public VObject
    {
    public:
        VSwapChain(const VulkanCore::VDevice &device,
                   const VulkanCore::VulkanInstance &instance);

        void Destroy() override;
        void RecreateSwapChain();

        ~VSwapChain() = default;

        const vk::SwapchainKHR &GetSwapChain() const { return m_swapChain; }
        const vk::SurfaceFormatKHR &GetSurfaceFormatKHR() const { return m_format; };
        const vk::Format &GetSwapChainFormat() const {return m_format.format;};
        const vk::Extent2D &GetExtent() const { return m_extent; };
        const vk::PresentModeKHR &GetPresentMode() const { return m_presentMode; };
        const std::vector<vk::Image>& GetSwapChainImages() const { return m_swapChainImages; };

    private:
        vk::SurfaceFormatKHR m_format;
        vk::Extent2D m_extent;
        vk::PresentModeKHR m_presentMode;

        vk::SwapchainKHR m_swapChain;

        std::vector<vk::Image> m_swapChainImages;

        const VulkanCore::VDevice &m_device;
        const VulkanCore::VulkanInstance &m_instance;
    private:
        void ChooseExtent();
        void ChooseFormat();
        void ChoosePresentMode();

        void CreateSwapChain();

        void RetrieveSwapChainImages();

    };

}


#endif //VSWAPCHAIN_HPP
