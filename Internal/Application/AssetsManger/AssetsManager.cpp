//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>
#include <unordered_map>
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Utils/LookUpTables.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Application/Utils/ModelExportImportUtils/ModelManagmentUtils.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Material/SkyBoxMaterial.hpp"
#include "EffectsLibrary/EffectsLibrary.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEnvLightGenerator/VEnvLightGenerator.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace ApplicationCore {
AssetsManager::AssetsManager(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary)
    : m_device(device)
    , m_meshDataManager(m_device.GetMeshDataManager())
    , m_transferOpsManager(device.GetTransferOpsManager())
    , m_materials()
    , m_effectsLibrary(effectsLibrary)
{
    CreateDefaultAssets();
}

void AssetsManager::DeleteAll()
{
    m_dummyImage->Destroy();
    for(auto& texture : m_textures)
    {
        texture.second->Destroy();
    }
    for(auto& mesh : m_meshes)
    {
        mesh.second->Destroy();
    }
    for(auto& hdr : m_HDRTextures)
    {
        if(hdr.second)
            hdr.second->Destroy();
    }
    for(auto& textureAsset : m_textures2)
    {
        textureAsset.second->Destroy();
    }
}

std::shared_ptr<StaticMesh> AssetsManager::GetDefaultMesh(EMeshGeometryType geometryType)
{
    auto result = m_preloadedMeshes.find(geometryType);
    // if they are loaded return the loaded result
    if(result != m_preloadedMeshes.end())
    {
        return std::make_shared<StaticMesh>(result->second, m_dummyMaterial);
    }
    // load them otherwise

    VulkanStructs::VMeshData data;
    switch(geometryType)
    {
        case Plane: {
            data = m_meshDataManager.AddMeshData(MeshData::planeVertices, MeshData::planeIndices);
            break;
        }
        case Sphere: {
            std::vector<Vertex>   vertices;
            std::vector<uint32_t> indices;
            MeshData::GenerateSphere(vertices, indices);

            data = m_meshDataManager.AddMeshData(vertices, indices);

            break;
        }
        case Cube: {
            data = m_meshDataManager.AddMeshData(MeshData::cubeVertices, MeshData::cubeIndices);

            break;
        }
        case Triangle: {
            data = m_meshDataManager.AddMeshData(MeshData::triangleVertices, MeshData::triangleIndices);
            break;
        }
        case Cross: {
            data = m_meshDataManager.AddMeshData(MeshData::crossVertices, MeshData::crossIndices);
            break;
        }
        case PostProcessQuad: {
            data = m_meshDataManager.AddMeshData(MeshData::fullscreenQuadVertices, MeshData::fullscreenQuadIndices);
            break;
        }
        case Arrow: {
            data = m_meshDataManager.AddMeshData(MeshData::arrowVertices, MeshData::arrowIndices);
            break;
        }
        case LinePlane: {
            data = m_meshDataManager.AddMeshData(MeshData::linePlaneVertices, MeshData::linePlaneIndices);
            break;
        }
        case SkyBox: {
            data = m_meshDataManager.AddMeshData(MeshData::skyboxVertices, MeshData::skyboxIndices);
            break;
        }
        default:;
            throw std::runtime_error("This geometry type is not supported !");
    }

    m_preloadedMeshes[geometryType] = data;

    m_meshDataManager.UpdateGPU(nullptr);

    return std::make_shared<StaticMesh>(data, m_dummyMaterial);
}

void AssetsManager::GetTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture, const std::string& path, bool saveToDisk)
{
    if(!m_textures2.contains(path))
    {
        m_textures2[path] =
            std::make_shared<ApplicationCore::VTextureAsset>(m_device, m_dummyImage, ETextureAssetType::Texture, path);
        m_texturesToSync.emplace_back(m_textures2[path]);
    }

    texture = m_textures2[path];
}

void AssetsManager::GetTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture,
                               const std::string&                               textureID,
                               TextureBufferInfo&                               data,
                               bool                                             saveToDisk)
{
    if(!m_textures2.contains(data.textureID))
    {
        m_textures2[data.textureID] =
            std::make_shared<ApplicationCore::VTextureAsset>(m_device, m_dummyImage, ETextureAssetType::Texture, data);
        m_texturesToSync.emplace_back(m_textures2[data.textureID]);
    }

    texture = m_textures2[data.textureID];
}

void AssetsManager::GetHDRTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture, const std::string& path, bool saveToDisk)
{
    if(!m_HDRTextures.contains(path))
    {
        m_HDRTextures[path] =
            std::make_shared<ApplicationCore::VTextureAsset>(m_device, m_dummyImage, ETextureAssetType::HDRTexture, path);
        m_texturesToSync.emplace_back(m_HDRTextures[path]);
    }
    texture = m_HDRTextures[path];
}


std::vector<std::shared_ptr<PBRMaterial>> AssetsManager::GetAllMaterials() const
{
    std::vector<std::shared_ptr<PBRMaterial>> materials;
    //materials.reserve(m_materials.size() +  m_editorIconsMaterials.size() );


    for(auto& material : m_materials)
    {
        if(material->IsSavable())
        {
            materials.emplace_back(material);
        }
    }

    for(auto& material : m_editorIconsMaterials)
    {
        if(material.second->IsSavable())
        {
            materials.emplace_back(material.second);
        }
    }

    materials.emplace_back(m_dummyMaterial);

    return materials;
}

void AssetsManager::AddMaterial(MaterialPaths& paths, std::shared_ptr<PBRMaterial> material)
{
    m_materials.emplace_back(material);
}

std::shared_ptr<StaticMesh> AssetsManager::GetEditorBilboardMesh(EEditorIcon icon)
{
    auto mesh = GetDefaultMesh(Plane);
    mesh->SetMaterial(m_editorIconsMaterials[icon]);

    mesh->SetName(IconToString(icon) + "##" + VulkanUtils::random_string(4));

    return std::move(mesh);
}

std::shared_ptr<SkyBoxMaterial> AssetsManager::AddSkyBoxMaterial(const std::filesystem::path& path)
{
    auto newSkyBox = std::make_shared<SkyBoxMaterial>(path, *this);
    newSkyBox->SetMaterialname(path.filename());
    m_skyBoxMaterials.emplace_back(std::move(newSkyBox));
    return m_skyBoxMaterials.back();
}

const std::vector<std::shared_ptr<ApplicationCore::SkyBoxMaterial>>& AssetsManager::GetAllSkyBoxMaterials() const
{
    return m_skyBoxMaterials;
}

std::map<EEffectType, std::shared_ptr<VulkanUtils::VEffect>> AssetsManager::GetEffects() const
{
    return m_effectsLibrary.effects;
}
std::map<EEffectType, std::shared_ptr<VulkanUtils::VRasterEffect>> AssetsManager::GetAllRasterEffects() const {
    std::map<EEffectType, std::shared_ptr<VulkanUtils::VRasterEffect>> collectedEffects;
    for (auto& effect :m_effectsLibrary.effects) {
        if (auto e = std::dynamic_pointer_cast<VulkanUtils::VRasterEffect>(effect.second))
        collectedEffects[effect.first] = e;
    }
    return collectedEffects;
}

void AssetsManager::AddModel(std::string path, std::vector<std::shared_ptr<ApplicationCore::SceneNode>>& model)
{
    m_models[path] = model;
}

std::vector<std::shared_ptr<ApplicationCore::SceneNode>> AssetsManager::GetModel(const std::string& path)
{
    auto it = m_models.find(path);
    if(it != m_models.end())
        return it->second;
    return {};
}


void AssetsManager::AddMesh(std::string meshName, std::shared_ptr<StaticMesh> mesh)
{
    if(!m_meshes.contains(meshName))
    {
        m_meshes[meshName] = mesh;
    }
}

void AssetsManager::DestroySkyBoxMaterial(const std::string& name)
{
    int i = 0;
    for(auto& skyBoxMat : m_skyBoxMaterials)
    {
        if(skyBoxMat->GetMaterialName() == name)
        {
            skyBoxMat->GetHDRTexture()->Destroy();
            m_skyBoxMaterials.erase(m_skyBoxMaterials.begin() + i);
            return;
        }
        i++;
    }
}

bool AssetsManager::Sync()
{
    int i = 0;
    if(!m_texturesToSync.empty())
    {
        for(auto& tex : m_texturesToSync)
        {
            if(tex)
            {
                if(tex->Sync())
                {
                    m_texturesToSync.erase(m_texturesToSync.begin() + (i));
                    //m_texturesToSync.shrink_to_fit();
                }
            }
            i++;
        }
        return false;
    }
    return false;
}

void AssetsManager::

    CreateDefaultAssets()
{
    auto dummyTextureData = ApplicationCore::LoadImage("Resources/DefaultTexture.jpg", false);
    m_dummyImage          = std::make_shared<VulkanCore::VImage2>(m_device, dummyTextureData);
    ;
    m_transferOpsManager.DestroyBuffer(m_dummyImage->GetImageStagingvBuffer(), true);

    m_dummyTexture = std::make_shared<ApplicationCore::VTextureAsset>(m_device, m_dummyImage);

    MaterialPaths paths{};
    m_dummyMaterial =
        std::make_shared<ApplicationCore::PBRMaterial>(m_effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::ForwardShader), paths, *this);

    MaterialPaths directionalLightBillboard{};
    directionalLightBillboard.DiffuseMapPath = "Resources/EditorIcons/light-directional.png";
    auto mat = std::make_shared<ApplicationCore::PBRMaterial>(m_effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::EditorBilboard),
                                                              directionalLightBillboard, *this);
    mat->SetMaterialname("Directional light editor billboard");
    m_editorIconsMaterials[EEditorIcon::DirectionalLight] = mat;

    MaterialPaths pointLightBillboard{};
    pointLightBillboard.DiffuseMapPath = "Resources/EditorIcons/light-point.png";

    mat = std::make_shared<ApplicationCore::PBRMaterial>(m_effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::EditorBilboard),
                                                         pointLightBillboard, *this);
    mat->SetMaterialname("Point light editor billboard");
    m_editorIconsMaterials[EEditorIcon::PointLight] = mat;

    MaterialPaths areaLightBillboard{};
    areaLightBillboard.DiffuseMapPath = "Resources/EditorIcons/light-area.png";
    mat = std::make_shared<ApplicationCore::PBRMaterial>(m_effectsLibrary.GetEffect<VulkanUtils::VRasterEffect>(EEffectType::EditorBilboard),
                                                         areaLightBillboard, *this);
    mat->SetMaterialname("Area light editor billboard");
    m_editorIconsMaterials[EEditorIcon::AreaLight] = mat;

    //=======================================
    // LTC TEXTURES
    //=======================================
    auto ltcTexture = std::make_shared<VulkanCore::VImage2>(m_device, MathUtils::LTC_ImageData);
    m_transferOpsManager.DestroyBuffer(ltcTexture->GetImageStagingvBuffer(), true);
    MathUtils::LUT.LTC = std::make_shared<VTextureAsset>(m_device, std::move(ltcTexture));

    ltcTexture = std::make_shared<VulkanCore::VImage2>(m_device, MathUtils::LTCInverse_ImageData);
    m_transferOpsManager.DestroyBuffer(ltcTexture->GetImageStagingvBuffer(), true);
    MathUtils::LUT.LTCInverse = std::make_shared<VTextureAsset>(m_device, std::move(ltcTexture));

    ltcTexture = std::make_shared<VulkanCore::VImage2>(m_device, )
    MathUtils::LUT.BlueNoise = std::make_shared<VulkanCore::VImage2>()

    Sync();
}

std::vector<TextureBufferView> AssetsManager::ReadBackAllTextures(std::vector<std::byte>& data)
{
    //prepare the data
    std::vector<TextureBufferView> views;
    views.reserve(m_textures.size());

    for(auto& texture : m_textures2)
    {
        if(texture.second->IsSavable())
        {
            TextureBufferView textureView;
            auto&             textureInfo = texture.second->GetHandle()->GetImageInfo();
            textureView.widht             = textureInfo.width;
            textureView.height            = textureInfo.height;
            textureView.path              = texture.first;
            textureView.size              = texture.second->GetHandle()->GetImageSizeBytes();
            textureView.textureAsset      = texture.second.get();
            views.emplace_back(textureView);
        }
    }

    return std::move(views);
}
}  // namespace ApplicationCore

// ApplicationCore
