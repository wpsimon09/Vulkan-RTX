//
// Created by wpsimon09 on 01/04/25.
//

#include "VEnvLightGenerator.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniform.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


VulkanUtils::VEnvLightGenerator::VEnvLightGenerator(const VulkanCore::VDevice&          device,
                                                    VulkanCore::VDescriptorLayoutCache& descLayoutCache)
    : m_device(device)
    , m_descLayoutChache(descLayoutCache)
{
    auto meshData = m_device.GetMeshDataManager().AddMeshData(ApplicationCore::MeshData::cubeVertices,
                                                              ApplicationCore::MeshData::cubeIndices);

    m_cube = std::make_unique<ApplicationCore::StaticMesh>(meshData, nullptr);

    m_graphicsCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    m_transferCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);

    m_graphicsCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_graphicsCmdPool);
    m_transferCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCmdPool);


    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    m_camptureViews             = {
        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    GenerateBRDFLut();

    const vk::Format format     = vk::Format::eR16G16B16A16Sfloat;
    const uint32_t   dimensions = 512;
    const uint32_t   mipLevels  = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

    //============================== Generate cube which is going ot be stored
    VulkanCore::VImage2CreateInfo dummyCubeMapCI;
    dummyCubeMapCI.channels    = 4;
    dummyCubeMapCI.format      = vk::Format::eR16G16B16A16Sfloat;
    dummyCubeMapCI.width       = 1;
    dummyCubeMapCI.height      = 1;
    dummyCubeMapCI.mipLevels   = 1;
    dummyCubeMapCI.arrayLayers = 6;  // six faces
    dummyCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

    m_dummyCubeMap = std::make_unique<VulkanCore::VImage2>(m_device, dummyCubeMapCI);
    RecordImageTransitionLayoutCommand(*m_dummyCubeMap, vk::ImageLayout::eShaderReadOnlyOptimal,
                                       vk::ImageLayout::eUndefined, m_device.GetTransferOpsManager().GetCommandBuffer());
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

const VulkanCore::VImage2& VulkanUtils::VEnvLightGenerator::GetIrradianceMap()
{
    return *m_irradianceMaps[m_currentHDR];
}

VulkanCore::VImage2* VulkanUtils::VEnvLightGenerator::GetIrradianceMapRaw()
{
    return m_irradianceMaps[m_currentHDR].get();
}

VulkanCore::VImage2* VulkanUtils::VEnvLightGenerator::GetDummyCubeMapRaw()
{
    return m_dummyCubeMap.get();
}

VulkanCore::VImage2* VulkanUtils::VEnvLightGenerator::GetPrefilterMapRaw()
{
    return m_prefilterMaps[m_currentHDR].get();
}

void VulkanUtils::VEnvLightGenerator::Generate(uint32_t                             currentFrame,
                                               std::shared_ptr<VulkanCore::VImage2> envMap,
                                               VulkanCore::VTimelineSemaphore&      renderingSemaphore)
{
    m_currentFrame = currentFrame;

    if(!envMap)
    {
        return;
    }
    //============================================
    // FIRST GENERATE CUBE MAP FROM hdr MAP
    //============================================
    m_currentHDR = envMap->GetID();

    if(!m_hdrCubeMaps.contains(envMap->GetID()))
    {
        HDRToCubeMap(envMap, renderingSemaphore);
    }
    if(!m_irradianceMaps.contains(envMap->GetID()))
    {
        CubeMapToIrradiance(envMap, renderingSemaphore);
    }
    if(!m_prefilterMaps.contains(envMap->GetID()))
    {
        {
            CubeMapToPrefilter(envMap, renderingSemaphore);
        }
    }
}

//==================================
// CUBE MAP GENERATION
//==================================
void VulkanUtils::VEnvLightGenerator::HDRToCubeMap(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                   VulkanCore::VTimelineSemaphore&      renderingSemaphore)
{

    {
        Utils::Logger::LogInfo("Generating IBL enviroment...");
        VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);
        const uint32_t                 dimensions = 1024;
        const uint32_t                 mipLevels  = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

        //============================== Generate cube which is going ot be stored
        VulkanCore::VImage2CreateInfo hdrCubeMapCI;
        hdrCubeMapCI.channels    = 4;
        hdrCubeMapCI.format      = vk::Format::eR32G32B32A32Sfloat;
        hdrCubeMapCI.width       = dimensions;
        hdrCubeMapCI.height      = dimensions;
        hdrCubeMapCI.mipLevels   = mipLevels;
        hdrCubeMapCI.arrayLayers = 6;  // six faces
        hdrCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

        std::unique_ptr<VulkanCore::VImage2> cubeMap;
        std::unique_ptr<VulkanCore::VImage2> renderTarget;

        // transition to colour attachment optimal
        m_graphicsCmdBuffer->BeginRecording();
        auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();

        CreateResources(cmdBuffer, cubeMap, renderTarget, hdrCubeMapCI, envGenerationSemaphore);

        m_hdrCubeMaps[envMap->GetID()] = std::move(cubeMap);
        ;

        auto& hdrCubeMap = m_hdrCubeMaps[envMap->GetID()];


        //=============================================================
        // START RENDERING
        // - renders hdr map to the offscreen buffer a
        // - copies ofcreen buffer the the one of the face of cube
        //=============================================================
        {
            VRasterEffect hdrToCubeMapEffect(m_device, "HDR Image to cube map", "Shaders/Compiled/HDRToCubeMap.vert.spv",
                                             "Shaders/Compiled/HDRToCubeMap.frag.spv", m_descLayoutChache);
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
            }viewProjPushBlock;


            // ======================= where we will render and copy to different arrray layers
            vk::RenderingAttachmentInfo renderAttachentInfo;
            renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
            renderAttachentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
            renderAttachentInfo.imageView        = renderTarget->GetImageView();
            renderAttachentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
            renderAttachentInfo.storeOp          = vk::AttachmentStoreOp::eStore;


            int          i = 0;
            vk::Viewport viewport{0, 0, (float)hdrCubeMapCI.width, (float)hdrCubeMapCI.height, 0.0f, 1.0f};

            hdrToCubeMapEffect.SetNumWrites(0, 1, 0);
            hdrToCubeMapEffect.WriteImage(m_currentFrame, 0, 0,
                                          envMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
            hdrToCubeMapEffect.ApplyWrites(m_currentFrame);

            for(int mip = 0; mip < mipLevels; mip++)
            {

                for(int face = 0; face < 6; face++)
                {

                    viewProjPushBlock.viewProj = m_camptureViews[face];

                    //================= configure rendering
                    viewport.width  = static_cast<float>(dimensions * std::pow(0.5f, mip));
                    viewport.height = static_cast<float>(dimensions * std::pow(0.5f, mip));
                    // ================ update data
                    hdrToCubeMapEffect.BindPipeline(cmdBuffer);


                    hdrToCubeMapEffect.BindDescriptorSet(cmdBuffer, m_currentFrame, 0);

                    vk::PushConstantsInfo pcInfo;
                    pcInfo.layout = hdrToCubeMapEffect.GetPipelineLayout();
                    pcInfo.offset = 0;
                    pcInfo.size = sizeof(PushBlock);
                    pcInfo.stageFlags = vk::ShaderStageFlagBits::eVertex ;
                    pcInfo.pValues = &viewProjPushBlock;

                    cmdBuffer.pushConstants2(pcInfo);

                    RenderToCubeMap(cmdBuffer, viewport, renderAttachentInfo);

                    //=================== transition layout to transfer src
                    RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eTransferSrcOptimal,
                                                       vk::ImageLayout::eColorAttachmentOptimal, *m_graphicsCmdBuffer);

                    //=================== cpy offscreen immage to the cueb map`s face
                    CopyResukt(cmdBuffer, renderTarget->GetImage(), hdrCubeMap->GetImage(), viewport.width,
                               viewport.height, mip, face);


                    //========================== transfer colour attachment back to rendering layout
                    RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eColorAttachmentOptimal,
                                                       vk::ImageLayout::eTransferSrcOptimal, *m_graphicsCmdBuffer);

                    //hdrPushBlock.Destory();
                    i++;
                }
            }


            //======================== transition the HDR image to shader read only
            RecordImageTransitionLayoutCommand(*hdrCubeMap, vk::ImageLayout::eShaderReadOnlyOptimal,
                                               vk::ImageLayout::eTransferDstOptimal, *m_graphicsCmdBuffer);

            std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                              vk::PipelineStageFlagBits::eTransfer};
            // waiting for value 2 because value 2 is signal by create resource command
            m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), envGenerationSemaphore.GetSemaphore(),
                                             envGenerationSemaphore.GetSemaphoreSubmitInfo(2, 4), waitStages.data());

            envGenerationSemaphore.CpuWaitIdle(4);

            envGenerationSemaphore.Reset();
            envGenerationSemaphore.Destroy();
            renderTarget->Destroy();
            hdrToCubeMapEffect.Destroy();


            Utils::Logger::LogSuccess("HDR Cube map generated");
        }
    }
}


//==================================
// IRRADIANCE GENERATION
//==================================
void VulkanUtils::VEnvLightGenerator::CubeMapToIrradiance(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                          VulkanCore::VTimelineSemaphore&      renderingSemaphore)
{
    {

        VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);

        //============================== Generate cube which is going ot be stored
        VulkanCore::VImage2CreateInfo irradianceCubeMapCI;
        irradianceCubeMapCI.channels    = 4;
        irradianceCubeMapCI.format      = vk::Format::eR16G16B16A16Sfloat;
        irradianceCubeMapCI.width       = 32;
        irradianceCubeMapCI.height      = 32;
        irradianceCubeMapCI.mipLevels   = 1;
        irradianceCubeMapCI.arrayLayers = 6;  // six faces
        irradianceCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;


        // transition to colour attachment optimal
        m_graphicsCmdBuffer->BeginRecording();
        auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();

        std::unique_ptr<VulkanCore::VImage2> cubeMap;
        std::unique_ptr<VulkanCore::VImage2> renderTarget;
        CreateResources(cmdBuffer, cubeMap, renderTarget, irradianceCubeMapCI, envGenerationSemaphore);

        m_irradianceMaps.insert(std::make_pair(envMap->GetID(), std::move(cubeMap)));
        auto& irradianceCubeMap = m_irradianceMaps[envMap->GetID()];

        //=============================================================
        // START RENDERING
        // - renders hdr map to the offscreen buffer a
        // - copies offcreen buffer the one of the face of cube
        //=============================================================
        {
            VRasterEffect cubeMapToIrradianceEffect(
                m_device, "HDR Image to cube map", "Shaders/Compiled/IrradianceMapImportanceSample.vert.spv",
                "Shaders/Compiled/IrradianceMapImportanceSample.frag.spv", m_descLayoutChache);
            cubeMapToIrradianceEffect.DisableStencil()
                .SetDisableDepthTest()
                .SetCullNone()
                .SetNullVertexBinding()
                .SetPiplineNoMultiSampling()
                .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
                .SetVertexInputMode(EVertexInput::PositionOnly);

            cubeMapToIrradianceEffect.BuildEffect();

            struct PushBlock
            {
                glm::mat4 viewProj;
            }viewProjPushBlock;

            // ======================= where we will render and copy to different arrray layers
            vk::RenderingAttachmentInfo renderAttachentInfo;
            renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
            renderAttachentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
            renderAttachentInfo.imageView        = renderTarget->GetImageView();
            renderAttachentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
            renderAttachentInfo.storeOp          = vk::AttachmentStoreOp::eStore;


            cubeMapToIrradianceEffect.SetNumWrites(0, 1);
            cubeMapToIrradianceEffect.WriteImage(
                m_currentFrame, 0, 0, m_hdrCubeMaps[envMap->GetID()]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));

            cubeMapToIrradianceEffect.ApplyWrites(m_currentFrame);


            for(int face = 0; face < 6; face++)
            {
                // ================ update data
                // create projection * view matrix that will be send to the  shader

                cubeMapToIrradianceEffect.BindPipeline(cmdBuffer);

                viewProjPushBlock.viewProj = m_camptureViews[face];

                vk::PushConstantsInfo pcInfo;
                pcInfo.layout = cubeMapToIrradianceEffect.GetPipelineLayout();
                pcInfo.offset = 0;
                pcInfo.size = sizeof(PushBlock);
                pcInfo.stageFlags = vk::ShaderStageFlagBits::eVertex ;
                pcInfo.pValues = &viewProjPushBlock;

                cmdBuffer.pushConstants2(pcInfo);


                cubeMapToIrradianceEffect.BindDescriptorSet(cmdBuffer, m_currentFrame, 0);

                //================== configure vieew port and scissors
                vk::Viewport viewport{0,    0,   (float)irradianceCubeMapCI.width, (float)irradianceCubeMapCI.height,
                                      0.0f, 1.0f};

                RenderToCubeMap(cmdBuffer, viewport, renderAttachentInfo);

                //=================== transition layout to transfer src
                RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eTransferSrcOptimal,
                                                   vk::ImageLayout::eColorAttachmentOptimal, *m_graphicsCmdBuffer);

                //=================== cpy offscreen immage to the cueb map`s face
                CopyResukt(cmdBuffer, renderTarget->GetImage(), irradianceCubeMap->GetImage(), viewport.width,
                           viewport.height, 0, face);


                //========================== transfer colour attachment back to rendering layout
                RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eColorAttachmentOptimal,
                                                   vk::ImageLayout::eTransferSrcOptimal, *m_graphicsCmdBuffer);

                //hdrPushBlock.Destory();
            }


            //======================== transition the HDR image to shader read only
            RecordImageTransitionLayoutCommand(*irradianceCubeMap, vk::ImageLayout::eShaderReadOnlyOptimal,
                                               vk::ImageLayout::eTransferDstOptimal, *m_graphicsCmdBuffer);

            std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                              vk::PipelineStageFlagBits::eTransfer};
            m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), envGenerationSemaphore.GetSemaphore(),
                                             envGenerationSemaphore.GetSemaphoreSubmitInfo(2, 4), waitStages.data());

            envGenerationSemaphore.CpuWaitIdle(4);

            envGenerationSemaphore.Reset();
            envGenerationSemaphore.Destroy();
            renderTarget->Destroy();

            cubeMapToIrradianceEffect.Destroy();

            //            hdrPushBlock.Destory();
            Utils::Logger::LogSuccess("Irradiance map generated");
        }
    }
}


//=====================================
// PREFILTER MAP
//=====================================
void VulkanUtils::VEnvLightGenerator::CubeMapToPrefilter(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                         VulkanCore::VTimelineSemaphore&      renderingSemaphore)
{
    VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);
    const vk::Format               format     = vk::Format::eR16G16B16A16Sfloat;
    const uint32_t                 dimensions = 512;
    const uint32_t                 mipLevels  = static_cast<uint32_t>(floor(log2(dimensions))) + 1;
    //============================== Generate cube which is going ot be stored
    VulkanCore::VImage2CreateInfo preffilterMapCI;
    preffilterMapCI.channels    = 4;
    preffilterMapCI.format      = vk::Format::eR16G16B16A16Sfloat;
    preffilterMapCI.width       = dimensions;
    preffilterMapCI.height      = dimensions;
    preffilterMapCI.mipLevels   = mipLevels;
    preffilterMapCI.arrayLayers = 6;  // six faces
    preffilterMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;


    std::unique_ptr<VulkanCore::VImage2> cubeMap;
    std::unique_ptr<VulkanCore::VImage2> renderTarget;

    m_graphicsCmdBuffer->BeginRecording();
    auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();


    CreateResources(cmdBuffer, cubeMap, renderTarget, preffilterMapCI, envGenerationSemaphore);

    m_prefilterMaps[envMap->GetID()] = std::move(cubeMap);
    auto& prefilterMap               = m_prefilterMaps[envMap->GetID()];

    //=============================================================
    // START RENDERING
    // - renders hdr map to the offscreen buffer a
    // - copies offcreen buffer the one of the face of cube
    //=============================================================
    {
        VRasterEffect hdrToPrefilterEffect(m_device, "HDR Image to cube map", "Shaders/Compiled/Prefilter.vert.spv",
                                           "Shaders/Compiled/Prefilter.frag.spv", m_descLayoutChache);
        hdrToPrefilterEffect.DisableStencil()
            .SetDisableDepthTest()
            .SetCullNone()
            .SetNullVertexBinding()
            .SetPiplineNoMultiSampling()
            .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
            .SetVertexInputMode(EVertexInput::PositionOnly);


        hdrToPrefilterEffect.BuildEffect();

        struct PushBlock
        {
            glm::mat4 viewProj;
            glm::vec4 params;  //x - roughness, yzw - padding
        }pushBlock;

        // ======================= where we will render and copy to different arrray layers
        vk::RenderingAttachmentInfo renderAttachentInfo;
        renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
        renderAttachentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
        renderAttachentInfo.imageView        = renderTarget->GetImageView();
        renderAttachentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
        renderAttachentInfo.storeOp          = vk::AttachmentStoreOp::eStore;

        std::vector<std::unique_ptr<VUniform<PushBlock>>> hdrPushBlocks(mipLevels * 6);
        for(auto& hdrPushBlock : hdrPushBlocks)
        {
            hdrPushBlock = std::make_unique<VUniform<PushBlock>>(m_device, true);
        }

        vk::Viewport viewport{0, 0, (float)preffilterMapCI.width, (float)preffilterMapCI.height, 0.0f, 1.0f};
        vk::Rect2D   scissors{{0, 0}, {(uint32_t)preffilterMapCI.width, (uint32_t)preffilterMapCI.height}};

        int i = 0;

        hdrToPrefilterEffect.SetNumWrites(0,1,0);
        hdrToPrefilterEffect.WriteImage(m_currentFrame, 0, 0,
                                        m_hdrCubeMaps[envMap->GetID()]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));

        hdrToPrefilterEffect.ApplyWrites(m_currentFrame);


        for(int mipLevel = 0; mipLevel < mipLevels; mipLevel++)
        {
            for(int face = 0; face < 6; face++)
            {

                hdrToPrefilterEffect.BindPipeline(cmdBuffer);

                hdrToPrefilterEffect.BindDescriptorSet(cmdBuffer, m_currentFrame, 0);

                // ================ update data
                // create projection * view matrix that will be send to the  shader
                pushBlock.viewProj = m_camptureViews[face];
                pushBlock.params.x = (float)mipLevel / float(mipLevels - 1);


                viewport.width  = static_cast<float>(dimensions * std::pow(0.5f, mipLevel));
                viewport.height = static_cast<float>(dimensions * std::pow(0.5f, mipLevel));

                vk::PushConstantsInfo pcInfo;
                pcInfo.layout = hdrToPrefilterEffect.GetPipelineLayout();
                pcInfo.offset = 0;
                pcInfo.size = sizeof(PushBlock);
                pcInfo.stageFlags = vk::ShaderStageFlagBits::eVertex ;
                pcInfo.pValues = &pushBlock;

                cmdBuffer.pushConstants2(pcInfo);

                RenderToCubeMap(cmdBuffer, viewport, renderAttachentInfo);

                //=================== transition layout to transfer src
                RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eTransferSrcOptimal,
                                                   vk::ImageLayout::eColorAttachmentOptimal, *m_graphicsCmdBuffer);

                CopyResukt(cmdBuffer, renderTarget->GetImage(), prefilterMap->GetImage(), viewport.width,
                           viewport.height, mipLevel, face);

                //========================== transfer colour attachment back to rendering layout
                RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eColorAttachmentOptimal,
                                                   vk::ImageLayout::eTransferSrcOptimal, *m_graphicsCmdBuffer);

                i++;
            }
        }


        //======================== transition the HDR image to shader read only
        RecordImageTransitionLayoutCommand(*prefilterMap, vk::ImageLayout::eShaderReadOnlyOptimal,
                                           vk::ImageLayout::eTransferDstOptimal, *m_graphicsCmdBuffer);

        std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                          vk::PipelineStageFlagBits::eTransfer};
        m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), envGenerationSemaphore.GetSemaphore(),
                                         envGenerationSemaphore.GetSemaphoreSubmitInfo(2, 4), waitStages.data());

        envGenerationSemaphore.CpuWaitIdle(4);

        envGenerationSemaphore.Reset();
        envGenerationSemaphore.Destroy();
        renderTarget->Destroy();
        for(auto& hdrPushBlock : hdrPushBlocks)
        {
            hdrPushBlock->Destory();
        }
        hdrToPrefilterEffect.Destroy();
        //            hdrPushBlock.Destory();
        Utils::Logger::LogSuccess("Prefilterred generated");
    }
}

//==================================
// BRDF LOOK UP TABLE GENERATION
//==================================
void VulkanUtils::VEnvLightGenerator::GenerateBRDFLut()
{
    VulkanCore::VTimelineSemaphore brdfGenerationSemaphore(m_device);
    //=======================================================
    // CREATE INFO FOR BRDF LOOK UP IMAGE
    //=======================================================
    VulkanCore::VImage2CreateInfo brdfCI;  // CI -create info
    brdfCI.channels = 2;
    brdfCI.format   = vk::Format::eR16G16Sfloat;
    brdfCI.width    = 512;
    brdfCI.height   = 512;
    brdfCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
    m_brdfLut = std::make_unique<VulkanCore::VImage2>(m_device, brdfCI);

    m_transferCmdBuffer->BeginRecording();

    RecordImageTransitionLayoutCommand(*m_brdfLut, vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eUndefined, *m_transferCmdBuffer);

    std::vector<vk::PipelineStageFlags> waitStages = {
        vk::PipelineStageFlagBits::eTopOfPipe,
    };

    m_transferCmdBuffer->EndAndFlush(m_device.GetTransferQueue(), brdfGenerationSemaphore.GetSemaphore(),
                                     brdfGenerationSemaphore.GetSemaphoreSubmitInfo(0, 2), waitStages.data());

    //==========================================================================
    // PREPARE FOR RENDERING
    //==========================================================================

    vk::RenderingAttachmentInfo brdfAttachmentCI;
    ;
    brdfAttachmentCI.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    brdfAttachmentCI.imageView   = m_brdfLut->GetImageView();
    brdfAttachmentCI.loadOp      = vk::AttachmentLoadOp::eClear;
    brdfAttachmentCI.resolveMode = vk::ResolveModeFlagBits::eNone;
    brdfAttachmentCI.storeOp     = vk::AttachmentStoreOp::eStore;
    brdfAttachmentCI.clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 1.f});

    //Create Effect that generates BRDF
    VRasterEffect brdfEffect(m_device, "BRDF Effect", "Shaders/Compiled/BRDFLut.vert.spv", "Shaders/Compiled/BRDFLut.frag.spv",
                             m_descLayoutChache);
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
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(brdfCI.width, brdfCI.height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &brdfAttachmentCI;
    ;
    renderingInfo.pDepthAttachment   = nullptr;
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
    m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), brdfGenerationSemaphore.GetSemaphore(),
                                     brdfGenerationSemaphore.GetSemaphoreSubmitInfo(2, 4), renderWaitStages.data());

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

    m_transferCmdBuffer->EndAndFlush(m_device.GetTransferQueue(), brdfGenerationSemaphore.GetSemaphore(),
                                     brdfGenerationSemaphore.GetSemaphoreSubmitInfo(4, 6), waitStages.data());

    brdfGenerationSemaphore.CpuWaitIdle(6);

    brdfGenerationSemaphore.Destroy();

    brdfEffect.Destroy();
}

void VulkanUtils::VEnvLightGenerator::RenderToCubeMap(const vk::CommandBuffer&     cmdBuffer,
                                                      vk::Viewport&                viewport,
                                                      vk::RenderingAttachmentInfo& attachment)
{
    vk::RenderingInfo hdrToCubeMapRenderingInfo;
    hdrToCubeMapRenderingInfo.colorAttachmentCount     = 1;
    hdrToCubeMapRenderingInfo.pColorAttachments        = &attachment;
    hdrToCubeMapRenderingInfo.renderArea.extent.width  = viewport.width;
    hdrToCubeMapRenderingInfo.renderArea.extent.height = viewport.height;
    hdrToCubeMapRenderingInfo.renderArea.offset        = 0;
    hdrToCubeMapRenderingInfo.renderArea.offset        = 0;
    hdrToCubeMapRenderingInfo.layerCount               = 1;

    cmdBuffer.beginRendering(hdrToCubeMapRenderingInfo);


    cmdBuffer.bindVertexBuffers(0, {m_cube->GetMeshData()->vertexData.buffer}, {0});
    cmdBuffer.bindIndexBuffer(m_cube->GetMeshData()->indexData.buffer, 0, vk::IndexType::eUint32);


    //================== configure vieew port and scissors
    cmdBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0}, {(uint32_t)viewport.width, (uint32_t)viewport.height}};
    cmdBuffer.setScissor(0, 1, &scissors);
    cmdBuffer.setStencilTestEnable(false);


    //==================== Render the cube as a sky box
    cmdBuffer.drawIndexed(m_cube->GetMeshData()->indexData.size / sizeof(uint32_t), 1,
                          m_cube->GetMeshData()->indexData.offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                          m_cube->GetMeshData()->vertexData.offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)), 0);

    cmdBuffer.endRendering();
}

void VulkanUtils::VEnvLightGenerator::CopyResukt(const vk::CommandBuffer& cmdBuffer,
                                                 const vk::Image&         src,
                                                 const vk::Image&         dst,
                                                 int                      w,
                                                 int                      h,
                                                 int                      m,
                                                 int                      f)
{
    //=================== cpy offscreen immage to the cueb map`s face
    vk::ImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    copyRegion.srcSubresource.layerCount     = 1;
    copyRegion.srcSubresource.mipLevel       = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcOffset                     = vk::Offset3D{0, 0, 0};

    copyRegion.dstSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
    copyRegion.dstSubresource.layerCount     = 1;
    copyRegion.dstSubresource.mipLevel       = m;
    copyRegion.dstSubresource.baseArrayLayer = f;
    copyRegion.dstOffset                     = vk::Offset3D{0, 0, 0};

    copyRegion.extent.width  = w;
    copyRegion.extent.height = h;
    copyRegion.extent.depth  = 1;

    cmdBuffer.copyImage(src, vk::ImageLayout::eTransferSrcOptimal, dst, vk::ImageLayout::eTransferDstOptimal, copyRegion);
}

void VulkanUtils::VEnvLightGenerator::CreateResources(const vk::CommandBuffer&              cmdBuffer,
                                                      std::unique_ptr<VulkanCore::VImage2>& cubeMap,
                                                      std::unique_ptr<VulkanCore::VImage2>& renderTarget,
                                                      VulkanCore::VImage2CreateInfo&        createInfo,
                                                      VulkanCore::VTimelineSemaphore&       semaphore)
{

    cubeMap = std::make_unique<VulkanCore::VImage2>(m_device, createInfo);

    //============================== Transefer layout to transfer ddestination
    RecordImageTransitionLayoutCommand(*cubeMap, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eUndefined, *m_graphicsCmdBuffer);

    //=============================== Create image that will be used to render into

    {
        VulkanCore::VImage2CreateInfo colourAttachemntCI;
        colourAttachemntCI.width  = createInfo.width;
        colourAttachemntCI.height = createInfo.height;
        colourAttachemntCI.format = createInfo.format;
        colourAttachemntCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

        renderTarget = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachemntCI);

        //============================ Transfer to transfer Src destination
        RecordImageTransitionLayoutCommand(*renderTarget, vk::ImageLayout::eColorAttachmentOptimal,
                                           vk::ImageLayout::eUndefined, *m_graphicsCmdBuffer);
    }

    //================ Transfer HDR cube map to be in transfer DST optimal
    std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    m_graphicsCmdBuffer->EndAndFlush(m_device.GetGraphicsQueue(), semaphore.GetSemaphore(),
                                     semaphore.GetSemaphoreSubmitInfo(0, 2), waitStages.data());
    semaphore.CpuWaitIdle(2);
    m_graphicsCmdBuffer->BeginRecording();
}


void VulkanUtils::VEnvLightGenerator::Destroy()
{
    m_brdfLut->Destroy();
    m_transferCmdPool->Destroy();
    m_graphicsCmdPool->Destroy();
    m_dummyCubeMap->Destroy();
    m_device.GetTransferOpsManager().ClearResources();
    for(auto& cubeMap : m_hdrCubeMaps)
    {
        if(cubeMap.second)
            cubeMap.second->Destroy();
    }
    for(auto& cubeMap : m_prefilterMaps)
    {
        if(cubeMap.second)
            cubeMap.second->Destroy();
    }
    for(auto& cubeMap : m_irradianceMaps)
    {
        if(cubeMap.second)
            cubeMap.second->Destroy();
    }
}
