//
// Created by wpsimon09 on 03/10/24.
//

#ifndef VPIPELINE_HPP
#define VPIPELINE_HPP
#include <functional>
#include <vulkan/vulkan.hpp>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include "vector"

namespace VulkanUtils {
class VRasterEffect;
}

namespace Renderer {
class RenderTarget;
}

namespace VulkanCore {
class VDescriptorSetLayout;
class VSwapChain;
class VDevice;
class VRenderPass;
class VShader;
class VCommandBuffer;

using Command = std::function<void(vk::CommandBuffer cmd)>;

class VGraphicsPipeline : public VObject
{

  public:
    VGraphicsPipeline(const VulkanCore::VDevice&              device,
                      const VulkanCore::VShader&              shaders,
                      const std::vector<vk::DescriptorSetLayout>&   descriptorSets,
                      const std::vector<vk::PushConstantRange>&   pushConstants);

    /**
         * Fills in all structs required to create pipeline. Structs can be modified with setters
         */
    void       Init();
    void       Destroy() override;
    const void RecordPipelineCommands(VulkanCore::VCommandBuffer& commandBuffer) const;
    const void AddCommand(const Command& command);

    const vk::GraphicsPipelineCreateInfo GetGraphicsPipelineCreateInfoStruct() const;
    const vk::Pipeline&                  GetPipelineInstance() const { return m_pipeline; }
    const vk::PipelineLayout&            GetPipelineLayout() const;
    ;
    const EPipelineType GetPipelineType() const { return m_pipelineType; };

    ~VGraphicsPipeline() = default;

  public:
    void CreateVertexInputBindingAndAttributes(EVertexInput input = Full);
    void CreateShaderStages();
    void CreatePrimitiveAssembler();
    void CreateDynamicViewPort();
    void CreateDynamicState();
    void CreateRasterizer();
    void CreateMultisampling();
    void CreateDepthStencil();
    void CreateColorBlend();
    void CreatePipelineLayout();
    void CreateRenderingInfo();

  private:
    const VulkanCore::VShader&              m_shaders;
    const VulkanCore::VDevice&              m_device;
    const std::vector<vk::DescriptorSetLayout>&   m_descriptorSets;
    const std::vector<vk::PushConstantRange>&     m_pushConstantRanges;

    // pipeline handler
    vk::Pipeline      m_pipeline;
    vk::PipelineCache m_pipelineCache;

    // pipeline commands
    std::vector<Command> m_pipelineCommands;

    // pipeline type
    EPipelineType m_pipelineType;

    //------------------------------
    //PIPELINE CREATE INFO VARIABLES
    //------------------------------
    std::vector<vk::Format> m_outputFormats;

  private:
    std::array<vk::PipelineShaderStageCreateInfo, 2> m_shaderStages;
    vk::VertexInputBindingDescription                m_vertexInputBindingDescription;
    std::vector<vk::VertexInputAttributeDescription> m_vertexInputAttributeDescription;
    vk::PipelineVertexInputStateCreateInfo           m_vertexInputState;
    vk::PipelineInputAssemblyStateCreateInfo         m_inputAssembly;
    vk::Viewport                                     m_viewport;
    vk::Rect2D                                       m_scissor;
    vk::PipelineViewportStateCreateInfo              m_viewportState;
    vk::PipelineDynamicStateCreateInfo               m_dynamicStateInfo;
    std::vector<vk::DynamicState>                    m_dynamicStates;
    vk::PipelineRasterizationStateCreateInfo         m_rasterizer;
    vk::PipelineMultisampleStateCreateInfo           m_multisampling;
    vk::PipelineDepthStencilStateCreateInfo          m_depthStencil;
    vk::PipelineColorBlendAttachmentState            m_colorBlendAttachmentState;
    vk::PipelineColorBlendStateCreateInfo            m_colorBlendState;
    vk::PipelineLayout                               m_pipelineLayout;
    vk::PipelineRenderingCreateInfo                  m_renderingCreateInfo;

  public:
    void SetCreatedPipeline(vk::Pipeline pipeline)
    {
        assert(pipeline);
        m_pipeline = pipeline;
    }

    void SetColourOutputFormat(vk::Format format);


    void SetVertexInputBindingDescription(const vk::VertexInputBindingDescription& vertexInputBindingDescription)
    {
        m_vertexInputBindingDescription = vertexInputBindingDescription;
    }

    void SetVertexInputAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& vertexInputAttributesDescription)
    {
        m_vertexInputAttributeDescription = vertexInputAttributesDescription;
    }

    void SetInputAssemply(const vk::PipelineInputAssemblyStateCreateInfo& inputAssemebly)
    {
        m_inputAssembly = inputAssemebly;
    }

    void SetDynamicStateInfo(const vk::PipelineDynamicStateCreateInfo& dynamicStateInfo)
    {
        m_dynamicStateInfo = dynamicStateInfo;
    }

    void AddDynamicState(const vk::DynamicState& newDynamicState) { m_dynamicStates.emplace_back(newDynamicState); }

    void SetRasterizer(const vk::PipelineRasterizationStateCreateInfo& rasterizer) { m_rasterizer = rasterizer; }

    void SetMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling)
    {
        m_multisampling = multisampling;
    }

    void SetDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& m_depth_stencil)
    {
        m_depthStencil = m_depth_stencil;
    }

    void SetColorBlendAttachmentState(const vk::PipelineColorBlendAttachmentState& m_color_blend_attachment_state)
    {
        m_colorBlendAttachmentState = m_color_blend_attachment_state;
    }

    void SetColorBlendState(const vk::PipelineColorBlendStateCreateInfo& m_color_blend_state)
    {
        m_colorBlendState = m_color_blend_state;
    }

    void SetPipelineLayout(const vk::PipelineLayout& m_pipeline_layout) { m_pipelineLayout = m_pipeline_layout; }

    void SetPipelineType(EPipelineType type) { m_pipelineType = type; }

    void SetPrimitiveTopology(vk::PrimitiveTopology topology) { m_inputAssembly.topology = topology; }

    void SetCullMode(vk::CullModeFlags cullMode) { m_rasterizer.cullMode = cullMode; }

    void SetPolygonMode(vk::PolygonMode polygonMode) { m_rasterizer.polygonMode = polygonMode; }

    void DisableDepthTest()
    {
        m_depthStencil.depthTestEnable  = false;
        m_depthStencil.depthWriteEnable = false;
    }

    void SetLineWidth(uint32_t lineWidth) { m_rasterizer.lineWidth = lineWidth; }

    void DisableDepthWrite() { m_depthStencil.depthWriteEnable = false; }

    void SetStencilState(vk::StencilOpState& stencilState);


    void EnableBlendingAdditive();
    void EnableBlendingAlpha();


  private:
    friend VulkanUtils::VRasterEffect;
};
}  // namespace VulkanCore


#endif  //VPIPELINE_HPP
