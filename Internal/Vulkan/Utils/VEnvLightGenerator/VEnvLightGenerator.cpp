//
// Created by wpsimon09 on 01/04/25.
//

#include "VEnvLightGenerator.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/VEffect/VComputeEffect.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniform.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


namespace VulkanCore {
class VTimelineSemaphore2;
}
VulkanUtils::VEnvLightGenerator::VEnvLightGenerator(const VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache)
    : m_device(device)
    , m_descLayoutChache(descLayoutCache)
{
    auto meshData = m_device.GetMeshDataManager().AddMeshData(ApplicationCore::MeshData::cubeVertices,
                                                              ApplicationCore::MeshData::cubeIndices);

    m_cube = std::make_unique<ApplicationCore::StaticMesh>(meshData, nullptr);

    m_graphicsCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Graphics);
    m_transferCmdPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);


    m_graphicsCmdBuffer = &m_device.GetTransferOpsManager().GetCommandBuffer();
    m_transferCmdBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *m_transferCmdPool);


    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    m_camptureViews             = {
        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

        captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    //============================================
    // Create all shader effects up-front
    m_hdrToCubeMapEffect =
        std::make_unique<VulkanUtils::VRasterEffect>(m_device, "HDR Image to cube map", "Shaders/Compiled/HDRToCubeMap.vert.spv",
                                                     "Shaders/Compiled/HDRToCubeMap.frag.spv", m_descLayoutChache);
    m_hdrToCubeMapEffect->DisableStencil()
        .SetDisableDepthTest()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetPiplineNoMultiSampling()
        .SetColourOutputFormat(vk::Format::eR32G32B32A32Sfloat)
        .SetVertexInputMode(EVertexInput::PositionOnly);

    m_hdrToCubeMapEffect->BuildEffect();

    //-----------------------------------------------

    m_hdrToIrradianceEffect =
        std::make_unique<VRasterEffect>(m_device, "Irradiance map generation", "Shaders/Compiled/IrradianceMapImportanceSample.vert.spv",
                                        "Shaders/Compiled/IrradianceMapImportanceSample.frag.spv", m_descLayoutChache);
    m_hdrToIrradianceEffect->DisableStencil()
        .SetDisableDepthTest()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetPiplineNoMultiSampling()
        .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
        .SetVertexInputMode(EVertexInput::PositionOnly);

    m_hdrToIrradianceEffect->GetReflectionData()->Print();

    m_hdrToIrradianceEffect->BuildEffect();

    //------------------------------------------------

    m_prefilterEffect = std::make_unique<VRasterEffect>(m_device, "Prefilter map generation", "Shaders/Compiled/Prefilter.vert.spv",
                                                        "Shaders/Compiled/Prefilter.frag.spv", m_descLayoutChache);
    m_prefilterEffect->DisableStencil()
        .SetDisableDepthTest()
        .SetCullNone()
        .SetNullVertexBinding()
        .SetPiplineNoMultiSampling()
        .SetColourOutputFormat(vk::Format::eR16G16B16A16Sfloat)
        .SetVertexInputMode(EVertexInput::PositionOnly);

    m_prefilterEffect->GetReflectionData()->Print();

    m_prefilterEffect->BuildEffect();


    //------------------------------------------------

    m_temporaryRenderTargets.resize(EIBLAttachmetn::IBLAttachmentCount);
    CreateTempRenderTargets(EIBLAttachmetn::HDR, )

    //================================================

    GenerateBRDFLutCompute();

    const vk::Format format     = vk::Format::eR16G16B16A16Sfloat;
    const uint32_t   dimensions = 512;
    const uint32_t   mipLevels  = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

    //============================== Generate cube which is going to be stored
    VulkanCore::VImage2CreateInfo dummyCubeMapCI;
    dummyCubeMapCI.channels    = 4;
    dummyCubeMapCI.format      = vk::Format::eR16G16B16A16Sfloat;
    dummyCubeMapCI.width       = 1;
    dummyCubeMapCI.height      = 1;
    dummyCubeMapCI.mipLevels   = 1;
    dummyCubeMapCI.arrayLayers = 6;  // six faces
    dummyCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

    m_dummyCubeMap                                = std::make_unique<VulkanCore::VImage2>(m_device, dummyCubeMapCI);
    VulkanUtils::VBarrierPosition barrierPosition = {
        {}, {}, vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eColorAttachmentRead};

    VulkanUtils::PlaceImageMemoryBarrier2(*m_dummyCubeMap, m_device.GetTransferOpsManager().GetCommandBuffer(),
                                          vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, barrierPosition);
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
                                               VulkanCore::VTimelineSemaphore2&      renderingSemaphore)
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
                                                   VulkanCore::VTimelineSemaphore2&      renderingSemaphore)
{

    {
        Utils::Logger::LogInfo("Generating IBL enviroment...");
        VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);


        // transition to colour attachment optimal
        auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();

        std::unique_ptr<VulkanCore::VImage2> cubeMap;

        const uint32_t                 dimensions = 1024;
        const uint32_t                 mipLevels  = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

        if (m_temporaryRenderTargets[EIBLAttachmetn::HDR] == nullptr) {
            CreateTempRenderTargets(EIBLAttachmetn::HDR, dimensions, dimensions, vk::Format::eR32G32B32A32Sfloat);
        }else {
            VulkanUtils::VBarrierPosition barrierPos = {
                vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferRead,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite
            };
            VulkanUtils::PlaceImageMemoryBarrier2(*m_temporaryRenderTargets[EIBLAttachmetn::HDR], *m_graphicsCmdBuffer, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal, barrierPos);
        }

        //============================== Generate cube which is going ot be stored
        VulkanCore::VImage2CreateInfo hdrCubeMapCI;
        hdrCubeMapCI.channels    = 4;
        hdrCubeMapCI.format      = vk::Format::eR32G32B32A32Sfloat;
        hdrCubeMapCI.width       = dimensions;
        hdrCubeMapCI.height      = dimensions;
        hdrCubeMapCI.mipLevels   = mipLevels;
        hdrCubeMapCI.arrayLayers = 6;  // six faces
        hdrCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

        cubeMap = std::make_unique<VulkanCore::VImage2>(m_device, hdrCubeMapCI);
        //============================== Transefer layout to transfer ddestination
        PlaceImageMemoryBarrier2(*cubeMap, *m_graphicsCmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, VulkanUtils::VImage_Undefined_ToTransferDst);

        m_hdrCubeMaps[envMap->GetID()] = std::move(cubeMap);


        auto& hdrCubeMap = m_hdrCubeMaps[envMap->GetID()];


        //=============================================================
        // START RENDERING
        // - renders hdr map to the offscreen buffer a
        // - copies ofcreen buffer the the one of the face of cube
        //=============================================================
        {
            struct PushBlock
            {
                glm::mat4 viewProj;
            } viewProjPushBlock;


            // ======================= where we will render and copy to different arrray layers
            vk::RenderingAttachmentInfo renderAttachentInfo;
            renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
            renderAttachentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
            renderAttachentInfo.imageView        = m_temporaryRenderTargets[EIBLAttachmetn::HDR]->GetImageView();
            renderAttachentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
            renderAttachentInfo.storeOp          = vk::AttachmentStoreOp::eStore;


            int          i = 0;
            vk::Viewport viewport{0, 0, (float)hdrCubeMapCI.width, (float)hdrCubeMapCI.height, 0.0f, 1.0f};

            m_hdrToCubeMapEffect->SetNumWrites(0, 1, 0);
            m_hdrToCubeMapEffect->WriteImage(m_currentFrame, 0, 0, envMap->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
            m_hdrToCubeMapEffect->ApplyWrites(m_currentFrame);

            for(int mip = 0; mip < mipLevels; mip++)
            {

                for(int face = 0; face < 6; face++)
                {

                    viewProjPushBlock.viewProj = m_camptureViews[face];

                    //================= configure rendering
                    viewport.width  = static_cast<float>(dimensions * std::pow(0.5f, mip));
                    viewport.height = static_cast<float>(dimensions * std::pow(0.5f, mip));
                    // ================ update data
                    m_hdrToCubeMapEffect->BindPipeline(cmdBuffer);


                    m_hdrToCubeMapEffect->BindDescriptorSet(cmdBuffer, m_currentFrame, 0);

                    vk::PushConstantsInfo pcInfo;
                    pcInfo.layout     = m_hdrToCubeMapEffect->GetPipelineLayout();
                    pcInfo.offset     = 0;
                    pcInfo.size       = sizeof(PushBlock);
                    pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;
                    pcInfo.pValues    = &viewProjPushBlock;

                    cmdBuffer.pushConstants2(pcInfo);

                    RenderToCubeMap(cmdBuffer, viewport, renderAttachentInfo);

                    //=================== transition layout to transfer src

                    PlaceImageMemoryBarrier2(*m_temporaryRenderTargets[EIBLAttachmetn::HDR], *m_graphicsCmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                             vk::ImageLayout::eTransferSrcOptimal, ColorAttachment_To_TransferSrc);

                    //=================== cpy offscreen immage to the cueb map`s face
                    CopyResukt(cmdBuffer, m_temporaryRenderTargets[EIBLAttachmetn::HDR]->GetImage(), hdrCubeMap->GetImage(), viewport.width,
                               viewport.height, mip, face);


                    //========================== transfer colour attachment back to rendering layout

                    PlaceImageMemoryBarrier2(*m_temporaryRenderTargets[EIBLAttachmetn::HDR], *m_graphicsCmdBuffer, vk::ImageLayout::eTransferSrcOptimal,
                                             vk::ImageLayout::eColorAttachmentOptimal, TransferSrc_To_ColorAttachment);

                    i++;
                }
            }


            //======================== transition the HDR image to shader read only

            PlaceImageMemoryBarrier2(*hdrCubeMap, *m_graphicsCmdBuffer, vk::ImageLayout::eTransferDstOptimal,
                                     vk::ImageLayout::eShaderReadOnlyOptimal, TransferDst_To_ReadOnly);



            Utils::Logger::LogSuccess("HDR Cube map generated");
        }
    }
}


//==================================
// IRRADIANCE GENERATION
//==================================
void VulkanUtils::VEnvLightGenerator::CubeMapToIrradiance(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                          VulkanCore::VTimelineSemaphore2&      renderingSemaphore)
{
    {

        VulkanCore::VTimelineSemaphore envGenerationSemaphore(m_device);

        if (m_temporaryRenderTargets[EIBLAttachmetn::IrradianceMap] == nullptr) {
            CreateTempRenderTargets(EIBLAttachmetn::HDR, 32, 32, vk::Format::eR16G16B16A16Sfloat);
        }else {
            VulkanUtils::VBarrierPosition barrierPos = {
                vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferRead,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite
            };
            VulkanUtils::PlaceImageMemoryBarrier2(*m_temporaryRenderTargets[EIBLAttachmetn::IrradianceMap], *m_graphicsCmdBuffer, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal, barrierPos);
        }

        //============================== Generate cube which is going ot be stored
        VulkanCore::VImage2CreateInfo irradianceCubeMapCI;
        irradianceCubeMapCI.channels    = 4;
        irradianceCubeMapCI.format      = vk::Format::eR16G16B16A16Sfloat;
        irradianceCubeMapCI.width       = 32;
        irradianceCubeMapCI.height      = 32;
        irradianceCubeMapCI.mipLevels   = 1;
        irradianceCubeMapCI.arrayLayers = 6;  // six faces
        irradianceCubeMapCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;


        auto& cmdBuffer = m_graphicsCmdBuffer->GetCommandBuffer();


        std::unique_ptr<VulkanCore::VImage2> cubeMap;

        m_irradianceMaps.insert(std::make_pair(envMap->GetID(), std::move(cubeMap)));
        auto& irradianceCubeMap = m_irradianceMaps[envMap->GetID()];

        //=============================================================
        // START RENDERING
        // - renders hdr map to the offscreen buffer
        // - copies offcreen buffer the one of the face of cube
        //=============================================================
        {
            struct PushBlock
            {
                glm::mat4 viewProj;
            } viewProjPushBlock;

            // ======================= where we will render and copy to different arrray layers
            vk::RenderingAttachmentInfo renderAttachentInfo;
            renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
            renderAttachentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
            renderAttachentInfo.imageView        = renderTarget->GetImageView();
            renderAttachentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
            renderAttachentInfo.storeOp          = vk::AttachmentStoreOp::eStore;


            m_hdrToIrradianceEffect->SetNumWrites(0, 1);
            m_hdrToIrradianceEffect->WriteImage(
                m_currentFrame, 0, 0, m_hdrCubeMaps[envMap->GetID()]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));

            m_hdrToIrradianceEffect->ApplyWrites(m_currentFrame);


            for(int face = 0; face < 6; face++)
            {
                // ================ update data
                // create projection * view matrix that will be send to the  shader

                m_hdrToIrradianceEffect->BindPipeline(cmdBuffer);

                viewProjPushBlock.viewProj = m_camptureViews[face];

                vk::PushConstantsInfo pcInfo;
                pcInfo.layout     = m_hdrToIrradianceEffect->GetPipelineLayout();
                pcInfo.offset     = 0;
                pcInfo.size       = sizeof(PushBlock);
                pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;
                pcInfo.pValues    = &viewProjPushBlock;

                cmdBuffer.pushConstants2(pcInfo);


                m_hdrToIrradianceEffect->BindDescriptorSet(cmdBuffer, m_currentFrame, 0);

                //================== configure vieew port and scissors
                vk::Viewport viewport{0,    0,   (float)irradianceCubeMapCI.width, (float)irradianceCubeMapCI.height,
                                      0.0f, 1.0f};

                RenderToCubeMap(cmdBuffer, viewport, renderAttachentInfo);

                //=================== transition layout to transfer src
                PlaceImageMemoryBarrier2(*renderTarget, *m_graphicsCmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                         vk::ImageLayout::eTransferSrcOptimal, ColorAttachment_To_TransferSrc);

                //=================== cpy offscreen immage to the cueb map`s face
                CopyResukt(cmdBuffer, renderTarget->GetImage(), irradianceCubeMap->GetImage(), viewport.width,
                           viewport.height, 0, face);

                //========================== transfer colour attachment back to rendering layout
                PlaceImageMemoryBarrier2(*renderTarget, *m_graphicsCmdBuffer, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal, TransferSrc_To_ColorAttachment);

                //hdrPushBlock.Destory();
            }


            //======================== transition the HDR image to shader read only
            PlaceImageMemoryBarrier2(*irradianceCubeMap, *m_graphicsCmdBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, TransferDst_To_ReadOnly);

            // TODO: add function that can also destry images

            //            hdrPushBlock.Destory();
            Utils::Logger::LogSuccess("Irradiance map generated");
        }
    }
}


//=====================================
// PREFILTER MAP
//=====================================
void VulkanUtils::VEnvLightGenerator::CubeMapToPrefilter(std::shared_ptr<VulkanCore::VImage2> envMap,
                                                         VulkanCore::VTimelineSemaphore2&      renderingSemaphore)
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


        struct PushBlock
        {
            glm::mat4 viewProj;
            glm::vec4 params;  //x - roughness, yzw - padding
        } pushBlock;

        // ======================= where we will render and copy to different arrray layers
        vk::RenderingAttachmentInfo renderAttachentInfo;
        renderAttachentInfo.clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
        renderAttachentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
        renderAttachentInfo.imageView        = renderTarget->GetImageView();
        renderAttachentInfo.loadOp           = vk::AttachmentLoadOp::eClear;
        renderAttachentInfo.storeOp          = vk::AttachmentStoreOp::eStore;

        vk::Viewport viewport{0, 0, (float)preffilterMapCI.width, (float)preffilterMapCI.height, 0.0f, 1.0f};
        vk::Rect2D   scissors{{0, 0}, {(uint32_t)preffilterMapCI.width, (uint32_t)preffilterMapCI.height}};

        int i = 0;

        m_prefilterEffect->SetNumWrites(0, 1, 0);
        m_prefilterEffect->WriteImage(m_currentFrame, 0, 0,
                                      m_hdrCubeMaps[envMap->GetID()]->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler10Mips));

        m_prefilterEffect->ApplyWrites(m_currentFrame);


        for(int mipLevel = 0; mipLevel < mipLevels; mipLevel++)
        {
            for(int face = 0; face < 6; face++)
            {

                m_prefilterEffect->BindPipeline(cmdBuffer);

                m_prefilterEffect->BindDescriptorSet(cmdBuffer, m_currentFrame, 0);

                // ================ update data
                // create projection * view matrix that will be send to the  shader
                pushBlock.viewProj = m_camptureViews[face];
                pushBlock.params.x = (float)mipLevel / float(mipLevels - 1);


                viewport.width  = static_cast<float>(dimensions * std::pow(0.5f, mipLevel));
                viewport.height = static_cast<float>(dimensions * std::pow(0.5f, mipLevel));

                vk::PushConstantsInfo pcInfo;
                pcInfo.layout     = m_prefilterEffect->GetPipelineLayout();
                pcInfo.offset     = 0;
                pcInfo.size       = sizeof(PushBlock);
                pcInfo.stageFlags = vk::ShaderStageFlagBits::eAll;
                pcInfo.pValues    = &pushBlock;

                cmdBuffer.pushConstants2(pcInfo);

                RenderToCubeMap(cmdBuffer,viewport, renderAttachentInfo);

                //=================== transition layout to transfer src
                PlaceImageMemoryBarrier2(*renderTarget, *m_graphicsCmdBuffer, vk::ImageLayout::eColorAttachmentOptimal,
                                         vk::ImageLayout::eTransferSrcOptimal, ColorAttachment_To_TransferSrc);

                CopyResukt(cmdBuffer, renderTarget->GetImage(), prefilterMap->GetImage(), viewport.width,
                           viewport.height, mipLevel, face);

                //========================== transfer colour attachment back to rendering layout
                PlaceImageMemoryBarrier2(*renderTarget, *m_graphicsCmdBuffer, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal, TransferSrc_To_ColorAttachment);

                i++;
            }
        }

        //======================== transition the HDR image to shader read only
        VulkanUtils::PlaceImageMemoryBarrier2(*prefilterMap, *m_graphicsCmdBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, TransferDst_To_ReadOnly);

        renderTarget->Destroy();


        //            hdrPushBlock.Destory();
        Utils::Logger::LogSuccess("Prefilterred generated");
    }
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


    cmdBuffer.bindVertexBuffers(0, {m_cube->GetMeshData()->vertexData->buffer}, {0});
    cmdBuffer.bindIndexBuffer(m_cube->GetMeshData()->indexData->buffer, 0, vk::IndexType::eUint32);


    //================== configure vieew port and scissors
    cmdBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissors{{0, 0}, {(uint32_t)viewport.width, (uint32_t)viewport.height}};
    cmdBuffer.setScissor(0, 1, &scissors);
    cmdBuffer.setStencilTestEnable(false);


    //==================== Render the cube as a sky box
    cmdBuffer.drawIndexed(m_cube->GetMeshData()->indexData->size / sizeof(uint32_t), 1,
                          m_cube->GetMeshData()->indexData->offset / static_cast<vk::DeviceSize>(sizeof(uint32_t)),
                          m_cube->GetMeshData()->vertexData->offset / static_cast<vk::DeviceSize>(sizeof(ApplicationCore::Vertex)),
                          0);

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


void VulkanUtils::VEnvLightGenerator::CreateTempRenderTargets(EIBLAttachmetn attachment, int width, int height, vk::Format format)
{
    VulkanCore::VImage2CreateInfo colourAttachemntCI;
    colourAttachemntCI.width  = width;
    colourAttachemntCI.height = height;
    colourAttachemntCI.format = format;
    colourAttachemntCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

    m_temporaryRenderTargets[attachment] = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachemntCI);

    //============================ Transfer to transfer Src destination
    PlaceImageMemoryBarrier2(*m_temporaryRenderTargets[attachment], *m_graphicsCmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, VulkanUtils::VImage_Undefined_ToColorAttachment);

}
//==================================
// BRDF LOOK UP TABLE GENERATION
//==================================
void VulkanUtils::VEnvLightGenerator::GenerateBRDFLutCompute()
{


    VulkanCore::VTimelineSemaphore brdfGenerationSemaphore(m_device);
    //=======================================================
    // CREATE INFO FOR BRDF LOOK UP IMAGE
    //=======================================================
    VulkanCore::VImage2CreateInfo brdfCI;  // CI -create info
    brdfCI.channels  = 2;
    brdfCI.format    = vk::Format::eR16G16Sfloat;
    brdfCI.width     = 512;
    brdfCI.height    = 512;
    brdfCI.layout    = vk::ImageLayout::eGeneral;
    brdfCI.isStorage = true;
    brdfCI.imageUsage |= vk::ImageUsageFlagBits::eStorage;
    m_brdfLut = std::make_unique<VulkanCore::VImage2>(m_device, brdfCI);

    VulkanUtils::VBarrierPosition barrierPos{
        {}, {},
        vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite
    };
    PlaceImageMemoryBarrier2(*m_brdfLut, *m_graphicsCmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, barrierPos);


    //==========================================================================
    // PREPARE FOR RENDERING
    //==========================================================================
    //Create Effect that generates BRDF
    VComputeEffect brdfCompute(m_device, "BRDF Lut compute", "Shaders/Compiled/BRDFLut.spv", m_descLayoutChache,
                               EShaderBindingGroup::ComputePostProecess);

    brdfCompute.BuildEffect();

    brdfCompute.GetReflectionData()->Print();

    for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
    {

        brdfCompute.SetNumWrites(0, 1, 0);
        brdfCompute.WriteImage(i, 0, 0, m_brdfLut->GetDescriptorImageInfo());
        brdfCompute.ApplyWrites(i);
    }

    //=============================================
    // RECORD COMMAND BUFFER
    //=============================================
    brdfCompute.BindPipeline(m_graphicsCmdBuffer->GetCommandBuffer());
    brdfCompute.BindDescriptorSet(m_graphicsCmdBuffer->GetCommandBuffer(), 0, 0);
    //=========================================
    // DISPATCH COMPUTE WORK
    //=========================================
    m_graphicsCmdBuffer->GetCommandBuffer().dispatch(m_brdfLut->GetImageInfo().width / 16, m_brdfLut->GetImageInfo().width / 16, 1);


    //=========================================
    // TRANSITION FROM GENERAL TO READ ONLY
    //=========================================
    barrierPos = {
        vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite,
        vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderSampledRead
    };
    PlaceImageMemoryBarrier2(*m_brdfLut, *m_graphicsCmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, barrierPos);

    brdfGenerationSemaphore.Destroy();


    //brdfEffect.Destroy();
}


void VulkanUtils::VEnvLightGenerator::Destroy()
{
    m_brdfLut->Destroy();
    m_transferCmdPool->Destroy();
    m_graphicsCmdPool->Destroy();
    m_dummyCubeMap->Destroy();
    m_hdrToIrradianceEffect->Destroy();
    m_hdrToCubeMapEffect->Destroy();
    m_prefilterEffect->Destroy();
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
