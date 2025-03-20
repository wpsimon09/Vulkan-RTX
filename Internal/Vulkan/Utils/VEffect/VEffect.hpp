//
// Created by wpsimon09 on 19/03/25.
//

#ifndef VEFFECT_HPP
#define VEFFECT_HPP
#include <variant>

#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"

namespace VulkanCore
{
    class VDevice;
}

namespace VulkanUtils {

class VEffect {
public:
    VEffect(const VulkanCore::VDevice& device, const VulkanCore::VShader& shader,const Renderer::RenderTarget &effectOutput, VulkanUtils::PushDescriptorVariant& descriptorSet);

    //=======================================
    // Effect building
    //=======================================
    void SetDisableDepthTest();
    void SetLineWidth(int lineWidth);
    void SetCullFrontFace();
    void SetDisableDepthWrite();
    void SetTopology(vk::PrimitiveTopology topology);
    void SetPolygonLine();
    void SetPolygonPoint();
    //=======================================

    void BuildEffect();
private:
    const VulkanCore::VShader& m_shader;
    std::unique_ptr<VulkanCore::VGraphicsPipeline> m_pipeline;
    const VulkanCore::VDevice& m_device;

    DescriptorSetTemplateVariant m_dstStruct;
    PushDescriptorVariant m_pushDescriptor;
};

} // VulkanUtils

#endif //VEFFECT_HPP
