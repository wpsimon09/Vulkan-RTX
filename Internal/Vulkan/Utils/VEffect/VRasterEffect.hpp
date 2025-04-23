//
// Created by wpsimon09 on 19/03/25.
//

#ifndef VRASTEREFFECT_HPP
#define VRASTEREFFECT_HPP
#include "VEffect.hpp"


#include <variant>

#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace VulkanCore::RTX {
struct RTXShaderPaths;
}
namespace Renderer {
class RenderTarget;
}

namespace VulkanCore {
class VShader;
class VDevice;
}  // namespace VulkanCore


namespace VulkanUtils {

class VRasterEffect:public VEffect
{
  public:
    VRasterEffect(const VulkanCore::VDevice&                          device,
            const std::string&                                  name,
            const VulkanCore::VShader&                          shader,
            std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& shaderResourceGroup);

    VRasterEffect(const VulkanCore::VDevice&                          device,
            const std::string&                                  name,
            const std::string&                                  vertex,
            const std::string&                                  fragment,
            std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet);

    VRasterEffect(const VulkanCore::VDevice&                          device,
            const VulkanCore::RTX::RTXShaderPaths&              rtShaderPaths,
            std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& resourceGroup);

        //=======================================
        // Effect building
        //=======================================
    VRasterEffect& SetDisableDepthTest();
    VRasterEffect& SetLineWidth(int lineWidth);
    VRasterEffect& SetCullFrontFace();
    VRasterEffect& SetCullNone();
    VRasterEffect& SetDisableDepthWrite();
    VRasterEffect& SetTopology(vk::PrimitiveTopology topology);
    VRasterEffect& SetPolygonLine();
    VRasterEffect& SetPolygonPoint();
    VRasterEffect& EnableAdditiveBlending();
    VRasterEffect& OutputHDR();
    VRasterEffect& SetDepthOpEqual();
    VRasterEffect& SetDepthOpLessEqual();
    VRasterEffect& SetFrontFaceClockWise();
    VRasterEffect& SetVertexInputMode(EVertexInput inputMode);
    VRasterEffect& SetStencilTestOutline();
    VRasterEffect& DisableStencil();
    VRasterEffect& SetDepthTestNever();
    VRasterEffect& SetColourOutputFormat(vk::Format format);
    VRasterEffect& SetPiplineNoMultiSampling();
    VRasterEffect& SetNullVertexBinding();
    VRasterEffect& DissableFragmentWrite();
    VRasterEffect& SetDepthOpLess();
    VRasterEffect& SetDepthOpAllways();


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
    friend bool operator==(const VRasterEffect& lhs, const VRasterEffect& rhs) { return lhs.m_ID == rhs.m_ID; }

    friend bool operator!=(const VRasterEffect& lhs, const VRasterEffect& rhs) { return !(lhs == rhs); }
};
}  // namespace VulkanUtils

#endif  //VRASTEREFFECT_HPP
