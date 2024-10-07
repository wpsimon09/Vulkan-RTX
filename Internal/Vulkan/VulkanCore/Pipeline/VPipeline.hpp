//
// Created by wpsimon09 on 03/10/24.
//

#ifndef VPIPELINE_HPP
#define VPIPELINE_HPP
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore
{
    class VSwapChain;
    class VDevice;
    class VShader;

    class VPipeline: public VObject
    {
    public:
        VPipeline(const VulkanCore::VDevice &device,const VulkanCore::VSwapChain &swapChain,const VulkanCore::VShader &shaders);

        /**
         * Fills in all structs required to create pipeline. Structs can be modified with setters
         */
        void Init();
        void Destroy() override;

        const vk::GraphicsPipelineCreateInfo GetPiplineCreateInfoStruct() const;
        ~VPipeline() = default;
    private:
        void CreateShaderStages();
        void CreateVertexInputBindingAndAttributes();
        void CreatePrimitiveAssembler();
        void CreateDynamicViewPort();
        void CreateDynamicState();
        void CreateRasterizer();
        void CreateMultisampling();
        void CreateDepthStencil();
        void CreateColorBlend();
        void CreatePipelineLayout(vk::DescriptorSetLayout* descriptorSet = nullptr, int descriptorCounts = 0) ;

    private:
        const VulkanCore::VShader &m_shaders;
        const VulkanCore::VDevice &m_device;
        const VulkanCore::VSwapChain &m_swapChain;

        // pipeline handler
        vk::Pipeline m_pipeline;
        vk::PipelineCache m_pipelineCache;

        //------------------------------
        //PIPELINE CREATE INFO VARIABLES
        //------------------------------
    private:
        std::array<vk::PipelineShaderStageCreateInfo, 2> m_shaderStages;
        vk::VertexInputBindingDescription m_vertexInputBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_vertexInputAttributeDescription;
        vk::PipelineVertexInputStateCreateInfo m_vertexInputState;
        vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly;
        vk::Viewport m_viewport;
        vk::Rect2D m_scissor;
        vk::PipelineViewportStateCreateInfo m_viewportState;
        vk::PipelineDynamicStateCreateInfo m_dynamicStateInfo;
        std::vector<vk::DynamicState> m_dynamicStates;
        vk::PipelineRasterizationStateCreateInfo m_rasterizer;
        vk::PipelineMultisampleStateCreateInfo m_multisampling;
        vk::PipelineDepthStencilStateCreateInfo m_depthStencil;
        vk::PipelineColorBlendAttachmentState m_colorBlendAttachmentState;
        vk::PipelineColorBlendStateCreateInfo m_colorBlendState;
        vk::PipelineLayout m_pipelineLayout;

    public:
        void SetVertexInputBindingDescription(
            const vk::VertexInputBindingDescription &vertexInputBindingDescription) {
            m_vertexInputBindingDescription = vertexInputBindingDescription;
        }

        void SetVertexInputAttributeDescription(
            const std::vector<vk::VertexInputAttributeDescription> &vertexInputAttributesDescription) {
            m_vertexInputAttributeDescription = vertexInputAttributesDescription;
        }

        void SetInputAssemply(const vk::PipelineInputAssemblyStateCreateInfo &inputAssemebly) {
            m_inputAssembly = inputAssemebly;
        }

        void SetDynamicStateInfo(const vk::PipelineDynamicStateCreateInfo &dynamicStateInfo) {
            m_dynamicStateInfo = dynamicStateInfo;
        }

        void AddDynamicState(const vk::DynamicState &newDynamicState) {
            m_dynamicStates.emplace_back(newDynamicState);
        }

        void SetRasterizer(const vk::PipelineRasterizationStateCreateInfo &rasterizer) {
            m_rasterizer = rasterizer;
        }

        void SetMultisampling(const vk::PipelineMultisampleStateCreateInfo &multisampling) {
            m_multisampling = multisampling;
        }

        void SetDepthStencil(const vk::PipelineDepthStencilStateCreateInfo &m_depth_stencil) {
            m_depthStencil = m_depth_stencil;
        }

        void SetColorBlendAttachmentState(
            const vk::PipelineColorBlendAttachmentState &m_color_blend_attachment_state) {
            m_colorBlendAttachmentState = m_color_blend_attachment_state;
        }

        void SetColorBlendState(const vk::PipelineColorBlendStateCreateInfo &m_color_blend_state) {
            m_colorBlendState = m_color_blend_state;
        }

        void SetPipelineLayout(const vk::PipelineLayout &m_pipeline_layout) {
            m_pipelineLayout = m_pipeline_layout;
        }
    };
}


#endif //VPIPELINE_HPP
