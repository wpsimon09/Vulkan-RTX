//
// Created by wpsimon09 on 16/08/2025.
//

#include "RenderPass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include <vulkan/vulkan_structs.hpp>
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


namespace Renderer {
RenderPass::RenderPass(const VulkanCore::VDevice& device, int width, int height)
    : m_device(device)
    , m_width(width)
    , m_height(height)
{
}

void RenderPass::Destroy()
{
    for(auto& renderTarget : m_renderTargets)
    {
        renderTarget->Destroy();
    }
}

RenderTarget2& RenderPass::GetRenderTarget(int index)
{
    return *m_renderTargets[index];
}
VulkanCore::VImage2& RenderPass::GetResolvedResult(int index)
{
    return m_renderTargets[index]->GetResolvedImage();
}
VulkanCore::VImage2& RenderPass::GetPrimaryResult(int index)
{
    return m_renderTargets[index]->GetPrimaryImage();
}

vk::DescriptorImageInfo RenderPass::GetPrimaryAttachemntDescriptorInfo(int attachment, vk::Sampler& sampler)
{
    assert(attachment != m_renderTargets.size());
    return m_renderTargets[attachment]->GetPrimaryImage().GetDescriptorImageInfo(sampler);
}

vk::DescriptorImageInfo RenderPass::GetPrimaryResolvedDescriptorInfo(int attachment, vk::Sampler& sampler)
{
    assert(attachment != m_renderTargets.size());
    return m_renderTargets[attachment]->GetResolvedImage().GetDescriptorImageInfo(sampler);
}

vk::DescriptorImageInfo RenderPass::GetPrimaryAttachemntDescriptorInfo(int attachment)
{
    assert(attachment != m_renderTargets.size());
    return m_renderTargets[attachment]->GetPrimaryImage().GetDescriptorImageInfo();
}

vk::DescriptorImageInfo RenderPass::GetPrimaryResolvedDescriptorInfo(int attachment)
{
    assert(attachment != m_renderTargets.size());
    return m_renderTargets[attachment]->GetResolvedImage().GetDescriptorImageInfo();
}

}  // namespace Renderer