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

    protected:
        const VulkanCore::VShader &m_shaders;
        const VulkanCore::VDevice &m_device;
        const VulkanCore::VSwapChain &m_swapChain;
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;

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
        std::vector<vk::DynamicState> m_dynamicStates;
    };
}


#endif //VPIPELINE_HPP
