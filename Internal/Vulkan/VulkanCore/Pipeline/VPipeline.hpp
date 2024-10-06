//
// Created by wpsimon09 on 03/10/24.
//

#ifndef VPIPELINE_HPP
#define VPIPELINE_HPP
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VSwapChain;
    class VDevice;
    class VShader;


    /**
     * This calss is a base class for every rasterization pipeline that will be created in this application.
     * New pipeline will inherit this class and override functions that needs to be different.
     * So for example if new pipeline has a different shader it will inherit this class and just override CreateShaderStagesFunction.
     */
    class VPipeline
    {
    public:
        VPipeline(const VulkanCore::VDevice &device,const VulkanCore::VSwapChain &swapChain,const VulkanCore::VShader &shaders);

        void DestoryPipeline();
        void CreatePipeline();

        ~VPipeline() = default;
    protected:
        virtual void CreateShaderStages();
        virtual void CreateVertexInputBindingAndAttributes();
        virtual void CreatePrimitiveAssembler();
        virtual void CreateDynamicViewPort();
        virtual void CreateDynamicState();
        virtual void CreateRasterizer();
        virtual void CreateMultisampling();
        virtual void CreateDepthStencil();
        virtual void CreateColorBlend();
        virtual void CreatePipelineLayout();

    protected:
        const VulkanCore::VShader &m_shaders;
        const VulkanCore::VDevice &m_device;
        const VulkanCore::VSwapChain &m_swapChain;

        // pipeline handler
        vk::Pipeline m_pipeline;
        vk::PipelineCache m_pipelineCache;


        //------------------------------
        //PIPELINE CREATE INFO VARIABLES
        //------------------------------
        std::array<vk::PipelineShaderStageCreateInfo, 2> m_shaderStages;
        vk::VertexInputBindingDescription m_vertexInputBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_vertexInputAttributeDescription;
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

    };
}


#endif //VPIPELINE_HPP
