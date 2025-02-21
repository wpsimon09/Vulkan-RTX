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
#include "Application/Utils/ModelExportImportUtils/ModelManagmentUtils.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"


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

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& path, bool saveToDisk)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(path))
        {
            if (!m_texturesToLoad.contains(path))
            {
                StartLoadingTexture(texture, path, saveToDisk);
                m_textures[path] = std::make_shared<VulkanCore::VImage>(m_device);
                m_textures[path]->SetSavable(saveToDisk);
            }
        }
        texture = m_textures[path];
    }

    void AssetsManager::GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
                                   TextureBufferInfo& data, bool saveToDisk)
    {
        // texture ID is a randomly generated string that is used to look up textures of which only data are available and no paths
        // this will most likely be used only within the editor that I plan to build in future
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_textures.contains(textureID))
        {
            if (!m_texturesToLoad.contains(textureID))
            {
                StartLoadingTexture(texture, textureID, data, saveToDisk);
                m_textures[textureID] = std::make_shared<VulkanCore::VImage>(m_device);
                m_textures[textureID]->SetPath(textureID);
                m_textures[textureID]->SetSavable(saveToDisk);

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
            if(material->IsSavable()){
                materials.emplace_back(material);
            }
        }

        for (auto &material : m_editorIconsMaterials)
        {
            if(material.second->IsSavable()){
                materials.emplace_back(material.second);
            }
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


    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texturePtr, const std::string& path, bool saveToDisk)
    {
        auto texture = std::async([this, path, saveToDisk]()
        {
            return LoadImage(path, saveToDisk);
        });
        m_texturesToLoad[path] = std::move(texture);
    }

    void AssetsManager::StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID,
                                            TextureBufferInfo& data, bool saveToDisk)
    {
        auto txt = std::async([this, textureID, data, saveToDisk]()
        {
            return LoadImage(data, textureID, saveToDisk);
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

    }
    
    std::vector<TextureBufferView> AssetsManager::ReadBackAllTextures(std::vector<std::byte>& data)
    {
        //this function will be called from separate therad so i will create new command pool 
        auto commandPool = std::make_unique<VulkanCore::VCommandPool>(m_device, EQueueFamilyIndexType::Transfer);
        //prepare the data 
        std::vector<TextureBufferView> views;
        views.reserve(m_textures.size());
        
        // size_t totalDataSize = 0;
        // vk::DeviceSize currentOffset = 0;
        // for(auto& texture: m_textures){
        //     totalDataSize += texture.second->GetSize();
        // }
        // data.resize(totalDataSize);
        // auto dstBuffer = VulkanUtils::CreateStagingBuffer(m_device, totalDataSize);

        // auto transferCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(m_device, *commandPool);

        // transferCommandBuffer->BeginRecording();

        for(auto& texture: m_textures){

            // texture.second->TransitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal,*transferCommandBuffer);
            
            // vk::BufferImageCopy cpyInfo;
            // cpyInfo.imageOffset = 0;
            // cpyInfo.bufferOffset = currentOffset;
            // cpyInfo.imageExtent.width = texture.second->GetWidth();
            // cpyInfo.imageExtent.height = texture.second->GetHeight();
            // cpyInfo.imageExtent.depth = 1; 
            
            // cpyInfo.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            // cpyInfo.imageSubresource.mipLevel = 0;
            // cpyInfo.imageSubresource.baseArrayLayer = 0;
            // cpyInfo.imageSubresource.layerCount = 1;

            
            if(texture.second->IsSavable()){
                TextureBufferView textureView;
                textureView.widht = texture.second->GetWidth();
                textureView.height = texture.second->GetHeight();
                textureView.path = texture.first;
                textureView.size = texture.second->GetSize();
                views.emplace_back(textureView);
                
            }
            //currentOffset += texture.second->GetSize();

            //transferCommandBuffer->GetCommandBuffer().copyImageToBuffer(texture.second->GetImage(), vk::ImageLayout::eTransferSrcOptimal, dstBuffer.m_stagingBufferVK, cpyInfo);

            //texture.second->TransitionImageLayout(vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,*transferCommandBuffer);
        }

        //transferCommandBuffer->EndAndFlush(m_device.GetTransferQueue());
        //m_device.GetTransferQueue().waitIdle(); 

        //memcpy(data.data(), dstBuffer.mappedPointer, totalDataSize);

        //vmaUnmapMemory(m_device.GetAllocator(), dstBuffer.m_stagingAllocation);
        //vmaDestroyBuffer(m_device.GetAllocator(), dstBuffer.m_stagingBufferVK, dstBuffer.m_stagingAllocation);

        return  std::move(views);
    }
}

// ApplicationCore
