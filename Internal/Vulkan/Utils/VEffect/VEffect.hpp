//
// Created by wpsimon09 on 19/03/25.
//

#ifndef VEFFECT_HPP
#define VEFFECT_HPP
#include <variant>

#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace Renderer {
class RenderTarget;
}

namespace VulkanCore {
class VShader;
class VDevice;
}  // namespace VulkanCore

inline int EffectIndexCounter = 0;

namespace VulkanUtils {

class VEffect
{
  public:
    VEffect(const VulkanCore::VDevice&                          device,
            const std::string&                                  name,
            const VulkanCore::VShader&                          shader,
            std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& shaderResourceGroup);

    VEffect(const VulkanCore::VDevice&                          device,
            const std::string&                                  name,
            const std::string&                                  vertex,
            const std::string&                                  fragment,
            std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet);

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
    VEffect& SetDepthOpEqual();
    VEffect& SetDepthOpLessEqual();
    VEffect& SetFrontFaceClockWise();
    VEffect& SetVertexInputMode(EVertexInput inputMode);
    VEffect& SetStencilTestOutline();
    VEffect& DisableStencil();
    VEffect& SetDepthTestNever();
    VEffect& SetColourOutputFormat(vk::Format format);
    VEffect& SetPiplineNoMultiSampling();
    VEffect& SetNullVertexBinding();
    VEffect& DissableFragmentWrite();
    VEffect& SetDepthOpLess();
    VEffect& SetDepthOpAllways();


    //=======================================

    std::string&                  GetName();
    DescriptorSetTemplateVariant& GetResrouceGroupStructVariant();
    void                          BuildEffect();
    vk::PipelineLayout            GetPipelineLayout();
    void                          BindPipeline(const vk::CommandBuffer& cmdBuffer);
    void                          Destroy();
    vk::DescriptorUpdateTemplate& GetUpdateTemplate();
    unsigned short                EvaluateRenderingOrder();
    int&                          GetID();
    EDescriptorLayoutStruct       GetLayoutStructType();

  private:
    const VulkanCore::VDevice&                         m_device;
    std::shared_ptr<VulkanUtils::VShaderResrouceGroup> m_resourceGroup;
    std::unique_ptr<VulkanCore::VGraphicsPipeline>     m_pipeline;
    std::string                                        m_name;
    std::optional<VulkanCore::VShader>                 m_shader;
    int                                                m_ID;

  private:
    friend bool operator==(const VEffect& lhs, const VEffect& rhs) { return lhs.m_ID == rhs.m_ID; }

    friend bool operator!=(const VEffect& lhs, const VEffect& rhs) { return !(lhs == rhs); }
};
}  // namespace VulkanUtils

#endif  //VEFFECT_HPP
