//
// Created by wpsimon09 on 01/04/25.
//

#include "VEnvLightGenerator.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniform.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


VulkanUtils::VEnvLightGenerator::VEnvLightGenerator(const VulkanCore::VDevice& device,
                                                    VulkanUtils::VPushDescriptorManager& pushDescriptorManager):
    m_device(device), m_pushDescriptorManager(pushDescriptorManager), m_cube(m_device.GetMeshDataManager().AddMeshData(
        ApplicationCore::MeshData::cubeVertices,
        ApplicationCore::MeshData::cubeIndices))
{
    m_graphicsCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    m_transferCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);

    m_graphicsCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_graphicsCmdPool);
    m_transferCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCmdPool);

    //GenerateBRDFLut();
}

const VulkanCore::VImage2& VulkanUtils::VEnvLightGenerator::GetBRDFLut()
{
    return *m_brdfLut;
}

void VulkanUtils::VEnvLightGenerator::Generate(
    std::shared_ptr<VulkanCore::VImage2> envMap,
    VulkanCore::VTimelineSemaphore& renderingSemaphore)
{
    if (!envMap) { return; }
    //============================================
    // FIRST GENERATE CUBE MAP FROM hdr MAP
    //============================================
    //TODO: later optimise with transfer queus for now i will just use the graphics one for everything
    {
        if (m_hdrCubeMaps.contains(envMap)) { return; }

        VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);

        VulkanCore::VImage2CreateInfo hdrCubeMapCI;
        hdrCubeMapCI.channels = 4;
        hdrCubeMapCI.format = vk::Format::eR32G32B32A32Sfloat;
        hdrCubeMapCI.width = 512;
        hdrCubeMapCI.height = 512;
        hdrCubeMapCI.mipLevels = 1;
        hdrCubeMapCI.arrayLayers = 6; // six faces
        hdrCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

        m_hdrCubeMaps[envMap] = std::make_unique<VulkanCore::VImage2>(m_device, hdrCubeMapCI);

        auto& hdrCubeMap = m_hdrCubeMaps[envMap];
        // transition to colour attachment optimal
        m_graphicsCmdBuffer->BeginRecording();

        RecordImageTransitionLayoutCommand(*hdrCubeMap, vk::ImageLayout::eTransferDstOptimal,
                                           vk::ImageLayout::eUndefined, *m_graphicsCmdBuffer);
        // make one image that will serve as a colour attachment, i will render to it and than transfer the results to the
        // HDR cubeMapCi
        std::unique_ptr<VulkanCore::VImage2> renderAttachment;
        {
            VulkanCore::VImage2CreateInfo colourAttachemntCI;
            colourAttachemntCI.width = 512;
            colourAttachemntCI.height = 512;
            colourAttachemntCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment |
                vk::ImageUsageFlagBits::eTransferSrc;

            renderAttachment = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachemntCI);

            RecordImageTransitionLayoutCommand(*renderAttachment, vk::ImageLayout::eColorAttachmentOptimal,
                                               vk::ImageLayout::eUndefined, *m_graphicsCmdBuffer);
        }

        // here we will render and copy to different arrray layers
        vk::RenderingAttachmentInfo renderAttachentInfo;
        renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
        renderAttachentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        renderAttachentInfo.imageView = renderAttachment->GetImageView();
        renderAttachentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        renderAttachentInfo.storeOp = vk::AttachmentStoreOp::eStore;

        // prepare colour attachment and rendering Cube map for rendering
        // colourAttachemnt - will be in colour attachemtn optimal layout
        // HDR result - will be in transfer dst optimal
        std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), envGenerationSemaphore.GetSemaphore(),
                                         envGenerationSemaphore.GetSemaphoreSubmitInfo(0, 2), waitStages.data());


        //=============================================================
        // START RENDERING
        // - renders hdr map to the faces of the cube creating cube ma
        //=============================================================
        {
            vk::RenderingInfo hdrToCubeMapRenderingInfo;
            hdrToCubeMapRenderingInfo.colorAttachmentCount = 1;
            hdrToCubeMapRenderingInfo.pColorAttachments = &renderAttachentInfo;
            hdrToCubeMapRenderingInfo.renderArea.extent.width = hdrCubeMapCI.width;
            hdrToCubeMapRenderingInfo.renderArea.extent.height = hdrCubeMapCI.height;
            hdrToCubeMapRenderingInfo.renderArea.offset = 0;
            hdrToCubeMapRenderingInfo.renderArea.offset = 0;

            VEffect hdrToCubeMapEffect(
                m_device, "HDR Image to cube map",
                "Shaders/Compiled/HDRToCubeMap.vert.spv",
                "Shaders/Compiled/HDRToCubeMap.frag.spv",
                m_pushDescriptorManager.GetPushDescriptor(EDescriptorLayoutStruct::UnlitSingleTexture));



            struct PushBlock
            {
                glm::mat4 viewProj;
            };

            for (int face = 0; face < 6; face++)
            {
                // ================ update data
                // create projection * view matrix that will be send to the  shader
                VUniform<PushBlock> hdrPushBlock(m_device);
                hdrPushBlock.GetUBOStruct().viewProj = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];
                hdrPushBlock.UpdateGPUBuffer(0);
                auto& updateStuct = std::get<UnlitSingleTexture>(hdrToCubeMapEffect.GetEffectUpdateStruct());
                updateStuct.buffer1 = hdrPushBlock.GetDescriptorBufferInfos()[0];
                updateStuct.texture2D_1 = envMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                //================= configure rendering

            }
        }
    }

    //m_transferCmdBuffer->EndAndFlush();
    // submit transfer operations all at once
    // ---wait on barrier----
    // submit graphics operations all at once
}


void VulkanUtils::VEnvLightGenerator::Destroy()
{
    m_brdfLut->Destroy();
    m_transferCmdPool->Destroy();
    m_graphicsCmdPool->Destroy();
}

void VulkanUtils::VEnvLightGenerator::GenerateBRDFLut()
{
    VulkanCore::VTimelineSemaphore brdfGenerationSemaphore(m_device);
    //=======================================================
    // CREATE INFO FOR BRDF LOOK UP IMAGE
    //=======================================================
    VulkanCore::VImage2CreateInfo brdfCI; // CI -create info
    brdfCI.channels = 2;
    brdfCI.format = vk::Format::eR16G16Sfloat;
    brdfCI.width = 512;
    brdfCI.height = 512;
    brdfCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
    m_brdfLut = std::make_unique<VulkanCore::VImage2>(m_device, brdfCI);

    m_transferCmdBuffer->BeginRecording();

    RecordImageTransitionLayoutCommand(*m_brdfLut, vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eUndefined, *m_transferCmdBuffer);

    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eTopOfPipe,
    };

    m_transferCmdBuffer->EndAndFlush(
        m_device.GetTransferQueue(),
        brdfGenerationSemaphore.GetSemaphore(),
        brdfGenerationSemaphore.GetSemaphoreSubmitInfo(0, 2),
        waitStages.data()
    );

    //==========================================================================
    // PREPARE FOR RENDERING
    //==========================================================================

    vk::RenderingAttachmentInfo brdfAttachmentCI;;
    brdfAttachmentCI.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    brdfAttachmentCI.imageView = m_brdfLut->GetImageView();
    brdfAttachmentCI.loadOp = vk::AttachmentLoadOp::eClear;
    brdfAttachmentCI.resolveMode = vk::ResolveModeFlagBits::eNone;
    brdfAttachmentCI.storeOp = vk::AttachmentStoreOp::eStore;
    brdfAttachmentCI.clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.f});

    //Create Effect that generates BRDF
    VEffect brdfEffect(m_device, "BRDF Effect",
                       "Shaders/Compiled/BRDFLut.vert.spv",
                       "Shaders/Compiled/BRDFLut.frag.spv",
                       m_pushDescriptorManager.GetPushDescriptor(EDescriptorLayoutStruct::Basic)
    );
    brdfEffect.SetColourOutputFormat(brdfCI.format)
              .DisableStencil()
              .SetDisableDepthTest()
              .SetCullNone()
              .SetNullVertexBinding()
              .SetPiplineNoMultiSampling();

    brdfEffect.BuildEffect();

    //=============================================
    // RECORD COMMAND BUFFER
    //=============================================
    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent = vk::Extent2D(brdfCI.width, brdfCI.height);
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &brdfAttachmentCI;;
    renderingInfo.pDepthAttachment = nullptr;
    renderingInfo.pStencilAttachment = nullptr;

    m_graphicsCmdBuffer->BeginRecording();
    auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();
    cmdBuffer.beginRendering(&renderingInfo);

    vk::Viewport viewport{0, 0, (float)brdfCI.width, (float)brdfCI.height, 0.0f, 1.0f};
    cmdBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0}, {(uint32_t)brdfCI.width, (uint32_t)brdfCI.height}};
    cmdBuffer.setScissor(0, 1, &scissors);
    cmdBuffer.setStencilTestEnable(false);

    brdfEffect.BindPipeline(cmdBuffer);
    cmdBuffer.draw(3, 1, 0, 0);

    cmdBuffer.endRendering();


    //=========================================
    // SUBMIT RENDERING WORK
    //=========================================
    std::vector<vk::PipelineStageFlags> renderWaitStages = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };
    m_graphicsCmdBuffer->EndAndFlush(
        m_device.GetGraphicsQueue(),
        brdfGenerationSemaphore.GetSemaphore(),
        brdfGenerationSemaphore.GetSemaphoreSubmitInfo(2, 4),
        renderWaitStages.data());

    brdfGenerationSemaphore.CpuWaitIdle(4);

    //=========================================
    // TRANSITION TO SHADER READ ONLY
    //=========================================
    m_transferCmdBuffer->BeginRecording();

    RecordImageTransitionLayoutCommand(*m_brdfLut, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       vk::ImageLayout::eColorAttachmentOptimal, *m_transferCmdBuffer);

    std::vector<vk::PipelineStageFlags> waitStagesToShaderReadOnly = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };

    m_transferCmdBuffer->EndAndFlush(
        m_device.GetTransferQueue(),
        brdfGenerationSemaphore.GetSemaphore(),
        brdfGenerationSemaphore.GetSemaphoreSubmitInfo(4, 6),
        waitStages.data()
    );

    brdfGenerationSemaphore.CpuWaitIdle(6);

    brdfEffect.Destroy();
}
