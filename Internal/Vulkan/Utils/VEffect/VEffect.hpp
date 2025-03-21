//
// Created by wpsimon09 on 19/03/25.
//

#ifndef VEFFECT_HPP
#define VEFFECT_HPP
#include <variant>

#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"

namespace Renderer
{
    class RenderTarget;
}

namespace VulkanCore
{
    class VShader;
    class VDevice;
}

namespace VulkanUtils {

class VEffect {
public:
    VEffect(const VulkanCore::VDevice& device, const VulkanCore::VShader& shader,
                const Renderer::RenderTarget& effectOutput, std::shared_ptr<VulkanUtils::VPushDescriptorSet>& descriptorSet);

    VEffect(const VulkanCore::VDevice& device, const std::string& vertex, const std::string& fragment,
            const Renderer::RenderTarget& effectOutput,
            std::shared_ptr<VulkanUtils::VPushDescriptorSet>& descriptorSet);

    //=======================================
    // Effect building
    //=======================================
    VEffect& SetDisableDepthTest();
    VEffect& SetLineWidth(int lineWidth);
    VEffect& SetCullFrontFace();
    VEffect& SetCullNone();
    VEffect& SetDisableDepthWrite();
    VEffect& SetTopology(vk::PrimitiveTopology topology);
    VEffect& SetPolygonLine();
    VEffect& SetPolygonPoint();
    VEffect& EnableAdditiveBlending();

    //=======================================

    DescriptorSetTemplateVariant& GetEffectUpdateStruct();
    void BuildEffect();

private:
    const VulkanCore::VDevice& m_device;
    std::shared_ptr<VulkanUtils::VPushDescriptorSet> m_descriptorSet;
    std::unique_ptr<VulkanCore::VGraphicsPipeline> m_pipeline;
};

} // VulkanUtils

#endif //VEFFECT_HPP
