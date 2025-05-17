//
// Created by wpsimon09 on 19/03/25.
//

#ifndef VRASTEREFFECT_HPP
#define VRASTEREFFECT_HPP
#include "VEffect.hpp"


#include <variant>

#include "Vulkan/Utils/VResrouceGroup/VResourceGroupManager.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace VulkanCore {
class VDescriptorLayoutCache;
}
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

class VRasterEffect : public VEffect
{
  public:
    VRasterEffect(const VulkanCore::VDevice&          device,
                  const std::string&                  name,
                  const VulkanCore::VShader&          shader,
                  VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                  EShaderBindingGroup                 bindingGroup = EShaderBindingGroup::ForwardUnlit);

    VRasterEffect(const VulkanCore::VDevice&          device,
                  const std::string&                  name,
                  const std::string&                  vertex,
                  const std::string&                  fragment,
                  VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                  EShaderBindingGroup                 bindingGroup = EShaderBindingGroup::ForwardUnlit);

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
    void               BuildEffect() override;
    vk::PipelineLayout GetPipelineLayout() override;
    void               BindPipeline(const vk::CommandBuffer& cmdBuffer) override;
    void               Destroy() override;
    void               BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) override;


  private:
    std::unique_ptr<VulkanCore::VGraphicsPipeline> m_pipeline;
    std::optional<VulkanCore::VShader>             m_shader;

  private:
    friend bool operator==(const VRasterEffect& lhs, const VRasterEffect& rhs) { return lhs.m_ID == rhs.m_ID; }

    friend bool operator!=(const VRasterEffect& lhs, const VRasterEffect& rhs) { return !(lhs == rhs); }
};
}  // namespace VulkanUtils

#endif  //VRASTEREFFECT_HPP
