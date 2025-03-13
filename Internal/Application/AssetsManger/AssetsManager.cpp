//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>
#include <unordered_map>
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
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"

namespace ApplicationCore
{
    AssetsManager::AssetsManager(const VulkanCore::VDevice& device,VulkanUtils::VTransferOperationsManager& transferOpsManager, VulkanCore::MeshDatatManager& meshDataManager):
        m_device(device), m_meshDataManager(meshDataManager),m_transferOpsManager(transferOpsManager), m_materials()
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
        for (auto& textureAsset : m_textures2){
            textureAsset.second->Destroy();
        }
        //m_dummyTexture->Destroy();
        m_dummyImage->Destroy();
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

    void AssetsManager::GetTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture, const std::string& path, bool saveToDisk)
    {
        
        if (!m_textures2.contains(path)){
            m_textures2[path] = std::make_shared<ApplicationCore::VTextureAsset> (m_device, m_dummyImage, ETextureAssetType::Texture, path);
        }

        texture = m_textures2[path];
    }

    void AssetsManager::GetTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture, const std::string& textureID,
                                   TextureBufferInfo& data, bool saveToDisk)
    {

        if (!m_textures2.contains(data.textureID)){
            m_textures2[data.textureID] = std::make_shared<ApplicationCore::VTextureAsset> (m_device, m_dummyImage, ETextureAssetType::Texture, data);
        }

        texture = m_textures2[data.textureID];
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

        materials.emplace_back(m_dummyMaterial);

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

    void AssetsManager::AddModel(std::string path, std::vector<std::shared_ptr<ApplicationCore::SceneNode>>& model)
    {
        m_models[path] = model;
    }

    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> AssetsManager::GetModel(const std::string& path)
    {
        auto it = m_models.find(path);
        if (it != m_models.end())
            return it->second;
        return {};
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
        for(auto& tex : m_textures2){
            tex.second->Sync();
        }
        Utils::Logger::LogInfoVerboseRendering("Nothing to sync...");
        return false;
    }

    void AssetsManager::CreateDefaultAssets()
    {
        auto dummyTextureData = ApplicationCore::LoadImage("Resources/DefaultTexture.jpg", false);
        m_dummyImage = std::make_shared<VulkanCore::VImage>(m_device, dummyTextureData);;

        m_dummyTexture = std::make_shared<ApplicationCore::VTextureAsset>(m_device,m_dummyImage);

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

        Sync();
    }
    
    std::vector<TextureBufferView> AssetsManager::ReadBackAllTextures(std::vector<std::byte>& data)
    {
    //prepare the data
        std::vector<TextureBufferView> views;
        views.reserve(m_textures.size());
        
        for(auto& texture: m_textures2){
            
            if(texture.second->IsSavable()){
                TextureBufferView textureView;
                textureView.widht = texture.second->GetHandle()->GetWidth();
                textureView.height = texture.second->GetHandle()->GetHeight();
                textureView.path = texture.first;
                textureView.size = texture.second->GetHandle()->GetSize();
                textureView.textureAsset = texture.second.get();
                views.emplace_back(textureView);
                
            }
        }


        return  std::move(views);
    }
}

// ApplicationCore
