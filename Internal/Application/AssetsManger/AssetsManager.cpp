//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>

#include "Application/Rendering/Material/Material.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Utils/LinearyTransformedCosinesValues.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"


namespace ApplicationCore
{
    AssetsManager::AssetsManager(const VulkanCore::VDevice& device, VulkanCore::MeshDatatManager& meshDataManager):
        m_device(device), m_meshDataManager(meshDataManager), m_materials()
    {
       CreateDefaultAssets();
    }

    void AssetsManager::DeleteAll()
    {


        for (auto& texture : m_textures)
        {
            texture.second->Destroy();
        }
        for (auto& mesh : m_meshes)
        {
            mesh.second->Destroy();
        }
        m_dummyTexture->Destroy();
    }

    std::shared_ptr<StaticMesh> AssetsManager::GetDefaultMesh(EMeshGeometryType geometryType)
    {
        auto result = m_preloadedMeshes.find(geometryType);
        // if they are loaded return the loaded result
        if (result != m_preloadedMeshes.end())
        {

            return std::make_shared<StaticMesh>(result->second, m_dummyMaterial);
        }
        // load them otherwise

        VulkanStructs::MeshData data;
        switch (geometryType)
        {
        case Plane:
            {
                data = m_meshDataManager.AddMeshData(MeshData::planeVertices, MeshData::planeIndices);
                break;
            }
        case Sphere:
            {
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                MeshData::GenerateSphere(vertices, indices);

                data = m_meshDataManager.AddMeshData(vertices, indices);

                break;
            }
        case Cube:
            {
                data = m_meshDataManager.AddMeshData(MeshData::cubeVertices, MeshData::cubeIndices);

                break;
            }
        case Triangle:
            {
                data = m_meshDataManager.AddMeshData(MeshData::triangleVertices, MeshData::triangleIndices);
                break;
            }
        case Cross:
            {
                data = m_meshDataManager.AddMeshData(MeshData::crossVertices, MeshData::crossIndices);
                break;
            }
        case PostProcessQuad:
            {
                data = m_meshDataManager.AddMeshData(MeshData::fullscreenQuadVertices,MeshData::fullscreenQuadIndices);
                break;
            }
        default: ;
            throw std::runtime_error("This geometry type is not supported !");
        }

        m_preloadedMeshes[geometryType] = data;

        m_meshDataManager.UpdateGPU(nullptr);

        return std::make_shared<StaticMesh>(data, m_dummyMaterial);
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(path))
        {
            if (!m_texturesToLoad.contains(path))
            {
                StartLoadingTexture(texture, path);
                m_textures[path] = std::make_shared<VulkanCore::VImage>(m_device);
            }
        }
        texture = m_textures[path];
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
                                   TextureBufferInfo& data)
    {
        // texture ID is a randomly generated string that is used to look up textures of which only data are available and no paths
        // this will most likely be used only within the editor that I plan to build in future
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(textureID))
        {
            if (!m_texturesToLoad.contains(textureID))
            {
                StartLoadingTexture(texture, textureID, data);
                m_textures[textureID] = std::make_shared<VulkanCore::VImage>(m_device);
            }
        }
        texture = m_textures[textureID];
    }


    std::vector<std::shared_ptr<Material>> AssetsManager::GetAllMaterials() const
    {
        std::vector<std::shared_ptr<Material>> materials;
        //materials.reserve(m_materials.size() +  m_editorIconsMaterials.size() );

        for (auto &material : m_materials)
        {
            materials.emplace_back(material);
        }

        for (auto &material : m_editorIconsMaterials)
        {
            materials.emplace_back(material.second);
        }

        return materials;
    }

    void AssetsManager::AddMaterial(MaterialPaths& paths, std::shared_ptr<Material> material)
    {
        m_materials.emplace_back(material);
    }

    std::shared_ptr<StaticMesh> AssetsManager::GetEditorBilboardMesh(EEditorIcon icon)
    {

        auto mesh = GetDefaultMesh(Plane);
        mesh->SetMaterial(m_editorIconsMaterials[icon]);

        mesh->SetName( IconToString(icon)+"##" + VulkanUtils::random_string(4));

        return std::move(mesh);
    }

    void AssetsManager::AddMesh(std::string meshName, std::shared_ptr<StaticMesh> mesh)
    {
        if (!m_meshes.contains(meshName))
        {
            m_meshes[meshName] = mesh;
        }
    }

    bool AssetsManager::Sync()
    {
        if (!m_texturesToLoad.empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            //for each texture that is being processed by separate thread
            for (auto it = m_texturesToLoad.begin(); it != m_texturesToLoad.end();)
            {
                if (it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    Utils::Logger::LogSuccess("Texture image loaded, swapping default texture for the loaded texture");
                    GlobalState::DisableLogging();
                    m_textures[it->first]->FillWithImageData<>(it->second.get(), true, true);
                    it = m_texturesToLoad.erase(it);
                    GlobalState::EnableLogging();
                }
                else
                {
                    ++it;
                }
            }
            return true;
        }
        Utils::Logger::LogInfoVerboseRendering("Nothing to sync...");
        return false;
    }


    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texturePtr, const std::string& path)
    {
        auto texture = std::async([this, path]()
        {
            return VulkanUtils::LoadImage(path);
        });
        m_texturesToLoad[path] = std::move(texture);
    }

    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
                                            TextureBufferInfo& data)
    {
        auto txt = std::async([this, textureID, data]()
        {
            return VulkanUtils::LoadImage(data, textureID);
        });
        m_texturesToLoad[textureID] = std::move(txt);
    }

    void AssetsManager::CreateDefaultAssets()
    {
        m_dummyTexture = std::make_shared<VulkanCore::VImage>(m_device);

        MaterialPaths paths{};
        m_dummyMaterial = std::make_shared<ApplicationCore::Material>(paths, *this);

        MaterialPaths directionalLightBillboard{};
        directionalLightBillboard.DiffuseMapPath = "Resources/EditorIcons/light-directional.png";
        auto mat = std::make_shared<ApplicationCore::Material>(directionalLightBillboard, *this);
        mat->SetMaterialname("Directional light editor billboard");
        m_editorIconsMaterials[EEditorIcon::DirectionalLight] = mat;

        MaterialPaths pointLightBillboard{};
        pointLightBillboard.DiffuseMapPath = "Resources/EditorIcons/light-point.png";
        mat = std::make_shared<ApplicationCore::Material>(pointLightBillboard, *this);
        mat->SetMaterialname("Point light editor billboard");
        m_editorIconsMaterials[EEditorIcon::PointLight] = mat;

        MaterialPaths areaLightBillboard{};
        areaLightBillboard.DiffuseMapPath = "Resources/EditorIcons/light-area.png";
        mat = std::make_shared<ApplicationCore::Material>(areaLightBillboard, *this);
        mat->SetMaterialname("Area light editor billboard");
        m_editorIconsMaterials[EEditorIcon::AreaLight] = mat;

        MaterialPaths suskoMaterial{};
        suskoMaterial.DiffuseMapPath = "Resources/EditorIcons/susko.jpg";
        mat = std::make_shared<ApplicationCore::Material>(suskoMaterial, *this);
        mat->SetMaterialname("Susko material");
        m_materials.emplace_back(mat);

        //=======================================
        // LTC TEXTURES
        //=======================================
        auto ltcTexture = std::make_shared<VulkanCore::VImage>(m_device, 1, vk::Format::eR32G32B32A32Sfloat);
        ltcTexture->Resize(MathUtils::LTC_ImageData.widht,MathUtils::LTC_ImageData.height);
        ltcTexture->FillWithImageData<float>(MathUtils::LTC_ImageData);
        MathUtils::LUT.LTC = std::move(ltcTexture);

        //m_textures[MathUtils::LTC_ImageData.fileName] = std::move(ltcTexture);

        ltcTexture = std::make_shared<VulkanCore::VImage>(m_device, 1, vk::Format::eR32G32B32A32Sfloat);
        ltcTexture->Resize(MathUtils::LTCInverse_ImageData.widht,MathUtils::LTCInverse_ImageData.height);
        ltcTexture->FillWithImageData<float>(MathUtils::LTCInverse_ImageData);
        MathUtils::LUT.LTCInverse = std::move(ltcTexture);


        //m_textures[MathUtils::LTCInverse_ImageData.fileName] = std::move(ltcTexture);


        //mat->GetTexture(ETextureType::Diffues) = m_textures[MathUtils::LTCInverse_ImageData.fileName];



    }
    
    std::pair<std::vector<std::byte>, std::vector<TextureBufferView>> AssetsManager::ReadBackAllTextures()
    {
        //prepare the data 
        std::vector<std::byte> data;
        std::vector<TextureBufferView> views(m_textures.size());
        size_t totalDataSize = 0;
        vk::DeviceSize currentOffset = 0;
        for(auto& texture: m_textures){
            totalDataSize += texture.second->GetSize();
        }
        data.reserve(totalDataSize);
        auto dstBuffer = VulkanUtils::CreateStagingBuffer(m_device, totalDataSize);

        auto transferCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, m_device.GetTransferCommandPool());

        transferCommandBuffer->BeginRecording();

        for(auto& texture: m_textures){
            
            auto imageTransferedSemaphore = std::make_unique<VulkanCore::VSyncPrimitive<vk::Semaphore>>(m_device);
            texture.second->TransitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);
        
            vk::BufferImageCopy cpyInfo;
            cpyInfo.imageOffset = 0;
            cpyInfo.bufferOffset = currentOffset;
            cpyInfo.imageExtent.width = texture.second->GetWidth();
            cpyInfo.imageExtent.height = texture.second->GetHeight();
            cpyInfo.imageExtent.depth = 1; 
        
            cpyInfo.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            cpyInfo.imageSubresource.mipLevel = 0;
            cpyInfo.imageSubresource.baseArrayLayer = 0;
            cpyInfo.imageSubresource.layerCount = 1;
            cpyInfo.bufferOffset = 0;

            transferCommandBuffer->GetCommandBuffer().copyImageToBuffer(texture.second->GetImage(), vk::ImageLayout::eTransferSrcOptimal, dstBuffer.m_stagingBufferVK, cpyInfo);

            currentOffset += texture.second->GetSize(); 
            
        }

        throw std::runtime_error("Not implemented yet");
    }
}

// ApplicationCore
