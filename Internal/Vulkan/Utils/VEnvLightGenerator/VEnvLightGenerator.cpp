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

    captureProjection =  glm::perspective(glm::radians(90.0f), 1.0f,0.1f, 10.0f);
    captureViews =
        {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)) * captureProjection,
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)) * captureProjection,
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)) * captureProjection,
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)) * captureProjection,
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)) * captureProjection,
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)) * captureProjection
        };

    GenerateBRDFLut();
}

const VulkanCore::VImage2& VulkanUtils::VEnvLightGenerator::GetBRDFLut()
{
    return *m_brdfLut;
}

VulkanCore::VImage2* VulkanUtils::VEnvLightGenerator::GetBRDFLutRaw()
{
    return m_brdfLut.get();
}

const VulkanCore::VImage2& VulkanUtils::VEnvLightGenerator::GetCubeMap()
{
    return *m_hdrCubeMaps[m_currentHDR];
}

VulkanCore::VImage2* VulkanUtils::VEnvLightGenerator::GetCubeMapRaw()
{
    return m_hdrCubeMaps[m_currentHDR].get();
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
    //TODO: later try to use compute shaders for all of this since they are faster
    m_currentHDR = envMap;
    if (!m_hdrCubeMaps.contains(envMap)) {HDRToCubeMap(envMap, renderingSemaphore);}
}

void VulkanUtils::VEnvLightGenerator::HDRToCubeMap(std::shared_ptr<VulkanCore::VImage2> envMap,
    VulkanCore::VTimelineSemaphore& renderingSemaphore)
{

    {

        VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);

        //============================== Generate cube which is going ot be stored
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
        auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();

        //============================== Transefer layout to transfer ddestination
        RecordImageTransitionLayoutCommand(*hdrCubeMap, vk::ImageLayout::eTransferDstOptimal,
                                           vk::ImageLayout::eUndefined, *m_graphicsCmdBuffer);

        //=============================== Create image that will be used to render into
        std::unique_ptr<VulkanCore::VImage2> renderAttachment;
        {
            VulkanCore::VImage2CreateInfo colourAttachemntCI;
            colourAttachemntCI.width = 512;
            colourAttachemntCI.height = 512;
            colourAttachemntCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment |
                vk::ImageUsageFlagBits::eTransferSrc;
            colourAttachemntCI.format = vk::Format::eR32G32B32A32Sfloat;

            renderAttachment = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachemntCI);

            //============================ Transfer to transfer Src destination
            RecordImageTransitionLayoutCommand(*renderAttachment, vk::ImageLayout::eColorAttachmentOptimal,
                                               vk::ImageLayout::eUndefined, *m_graphicsCmdBuffer);
        }

        //================ Transfer HDR cube map to be in transfer DST optimal
        std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), envGenerationSemaphore.GetSemaphore(),
                                         envGenerationSemaphore.GetSemaphoreSubmitInfo(0, 2), waitStages.data());
        envGenerationSemaphore.CpuWaitIdle(2);
        m_graphicsCmdBuffer->BeginRecording();

        //=============================================================
        // START RENDERING
        // - renders hdr map to the offscreen buffer a
        // - copies ofcreen buffer the the one of the face of cube
        //=============================================================
        {
            VEffect hdrToCubeMapEffect(
                m_device, "HDR Image to cube map",
                "Shaders/Compiled/HDRToCubeMap.vert.spv",
                "Shaders/Compiled/HDRToCubeMap.frag.spv",
                m_pushDescriptorManager.GetPushDescriptor(EDescriptorLayoutStruct::UnlitSingleTexture));
            hdrToCubeMapEffect.DisableStencil()
            .SetDisableDepthTest()
            .SetCullNone()
            .SetNullVertexBinding()
            .SetPiplineNoMultiSampling()
            .SetColourOutputFormat(vk::Format::eR32G32B32A32Sfloat)
            .SetVertexInputMode(EVertexInput::PositionOnly);


            hdrToCubeMapEffect.BuildEffect();

            struct PushBlock
            {
                glm::mat4 viewProj;
            };

            // ======================= where we will render and copy to different arrray layers
            vk::RenderingAttachmentInfo renderAttachentInfo;
            renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
            renderAttachentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            renderAttachentInfo.imageView = renderAttachment->GetImageView();
            renderAttachentInfo.loadOp = vk::AttachmentLoadOp::eClear;
            renderAttachentInfo.storeOp = vk::AttachmentStoreOp::eStore;

            VUniform<PushBlock> hdrPushBlock(m_device);
            for (int face = 0; face < 6; face++)
            {
                // ================ update data
                // create projection * view matrix that will be send to the  shader
                hdrPushBlock.GetUBOStruct().viewProj = captureViews[face];
                hdrPushBlock.UpdateGPUBuffer(0);
                hdrPushBlock.UpdateGPUBuffer(1);

                auto& updateStuct = std::get<UnlitSingleTexture>(hdrToCubeMapEffect.GetEffectUpdateStruct());
                updateStuct.buffer1 = hdrPushBlock.GetDescriptorBufferInfos()[0];
                updateStuct.texture2D_1 = envMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

                std::cout << "glm::mat4(\n";
                for (int i = 0; i < 4; ++i) {
                    std::cout << "  ";
                    for (int j = 0; j < 4; ++j) {
                        std::cout << hdrPushBlock.GetUBOStruct().viewProj[j][i] << (j < 3 ? ", " : "");
                    }
                    std::cout << "\n";
                }
                std::cout << ")" << std::endl;

                //================= configure rendering
                vk::RenderingInfo hdrToCubeMapRenderingInfo;
                hdrToCubeMapRenderingInfo.colorAttachmentCount = 1;
                hdrToCubeMapRenderingInfo.pColorAttachments = &renderAttachentInfo;
                hdrToCubeMapRenderingInfo.renderArea.extent.width = hdrCubeMapCI.width;
                hdrToCubeMapRenderingInfo.renderArea.extent.height = hdrCubeMapCI.height;
                hdrToCubeMapRenderingInfo.renderArea.offset = 0;
                hdrToCubeMapRenderingInfo.renderArea.offset = 0;
                hdrToCubeMapRenderingInfo.layerCount = 1;

                cmdBuffer.beginRendering(hdrToCubeMapRenderingInfo);

                hdrToCubeMapEffect.BindPipeline(cmdBuffer);

                cmdBuffer.bindVertexBuffers(0, {m_cube.vertexData.buffer}, {0});
                cmdBuffer.bindIndexBuffer(m_cube.indexData.buffer, 0, vk::IndexType::eUint32);


                //================== configure vieew port and scissors
                vk::Viewport viewport{0, 0, (float)hdrCubeMapCI.width, (float)hdrCubeMapCI.height, 0.0f, 1.0f};
                cmdBuffer.setViewport(0, 1, &viewport);

                vk::Rect2D scissors{{0, 0}, {(uint32_t)hdrCubeMapCI.width, (uint32_t)hdrCubeMapCI.height}};
                cmdBuffer.setScissor(0, 1, &scissors);
                cmdBuffer.setStencilTestEnable(false);

                cmdBuffer.pushDescriptorSetWithTemplateKHR(
                                        hdrToCubeMapEffect.GetUpdateTemplate(),
                                        hdrToCubeMapEffect.GetPipelineLayout(), 0,
                                        updateStuct, m_device.DispatchLoader);

                //==================== Render the cube as a sky box
                cmdBuffer.drawIndexed(
                    m_cube.indexData.size/sizeof(uint32_t),
                    1,
                    m_cube.indexData.offset/static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                        m_cube.vertexData.offset /static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                    0);

                cmdBuffer.endRendering();

                //=================== transition layout to transfer src
                RecordImageTransitionLayoutCommand(*renderAttachment, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal,  *m_graphicsCmdBuffer);

                //=================== cpy offscreen immage to the cueb map`s face
                vk::ImageCopy copyRegion{};
                copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcOffset = vk::Offset3D{0, 0, 0};

                copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstSubresource.mipLevel = 0;
                copyRegion.dstSubresource.baseArrayLayer = face;
                copyRegion.dstOffset = vk::Offset3D{0, 0, 0};

                copyRegion.extent.width = hdrCubeMapCI.width;
                copyRegion.extent.height = hdrCubeMapCI.height;
                copyRegion.extent.depth = 1;

                cmdBuffer.copyImage(
                    renderAttachment->GetImage(),
                    vk::ImageLayout::eTransferSrcOptimal,
                    hdrCubeMap->GetImage(),
                    vk::ImageLayout::eTransferDstOptimal,
                    copyRegion);

                //========================== transfer colour attachment back to rendering layout
                RecordImageTransitionLayoutCommand(*renderAttachment, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal, *m_graphicsCmdBuffer );
            }

            //======================== transition the HDR image to shader read only
            RecordImageTransitionLayoutCommand(*hdrCubeMap, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal, *m_graphicsCmdBuffer);

            std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::PipelineStageFlagBits::eTransfer};
            m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), envGenerationSemaphore.GetSemaphore(),
                                             envGenerationSemaphore.GetSemaphoreSubmitInfo(2, 4), waitStages.data());

            envGenerationSemaphore.CpuWaitIdle(4);

            envGenerationSemaphore.Reset();
            renderAttachment->Destroy();
            hdrToCubeMapEffect.Destroy();
            hdrPushBlock.Destory();

        }
    }
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
