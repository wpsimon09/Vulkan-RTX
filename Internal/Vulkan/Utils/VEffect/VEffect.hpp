//
// Created by wpsimon09 on 19/03/25.
//

#ifndef VEFFECT_HPP
#define VEFFECT_HPP
#include <variant>

#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

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
    VEffect(const VulkanCore::VDevice& device,const std::string& name,  const VulkanCore::VShader& shader, std::shared_ptr<VulkanUtils::VPushDescriptorSet>& descriptorSet);

    VEffect(const VulkanCore::VDevice& device,const std::string& name, const std::string& vertex, const std::string& fragment,
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
    VEffect& OutputHDR();

    //=======================================

    std::string&                        GetName();
    DescriptorSetTemplateVariant&       GetEffectUpdateStruct();
    void                                BuildEffect();
    vk::PipelineLayout                  GetPipelineLayout();
    void                                BindPipeline(const vk::CommandBuffer& cmdBuffer);
    void                                Destroy();
private:
    const VulkanCore::VDevice& m_device;
    std::shared_ptr<VulkanUtils::VPushDescriptorSet> m_descriptorSet;
    std::unique_ptr<VulkanCore::VGraphicsPipeline> m_pipeline;
    std::string m_name;
    std::optional<VulkanCore::VShader> shader;
};

} // VulkanUtils

#endif //VEFFECT_HPP
