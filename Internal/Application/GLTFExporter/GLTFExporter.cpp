#include "GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include <fastgltf/core.hpp>
#include <Application/Rendering/Scene/Scene.hpp>
#include <Application/Rendering/Scene/SceneNode.hpp>
#include <Application/AssetsManger/AssetsManager.hpp>
#include <Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp>
#include <Application/Utils/MathUtils.hpp>
#include <Application/AssetsManger/Utils/VTextureAsset.hpp>
#include <Vulkan/VulkanCore/VImage/VImage.hpp>

#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"


ApplicationCore::GLTFExporter::GLTFExporter(): m_lightInfo(GlobalVariables::lightInfoPath), m_lightInfoPath(GlobalVariables::configFolder / "Light.ini")
{
}

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path, Scene& scene,
                                                AssetsManager& assetsManager)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;


    Utils::Logger::LogInfoClient("Saving the scene !");
    auto start = std::chrono::high_resolution_clock::now();

    fastgltf::Asset asset;
    
    Utils::Logger::LogInfoClient("Parsing the scene...");


    m_textureDirectory = path / "textures";
    std::filesystem::create_directory(m_textureDirectory);
    ParseBuffers(asset, assetsManager);
    ParseTexture(asset, assetsManager);
    ParseMaterial(asset, assetsManager);
    ParseScene(scene.GetRootNode(), assetsManager, asset);
    OrganiseScene(asset);
    CreateScene(asset, scene);
    ParseLights(scene);


    //=============================================
    // WRITE TO FILE
    //=============================================
    Utils::Logger::LogSuccessClient("Scene parsed successfuly");

    std::filesystem::path p = path;
    std::filesystem::path datapath = "data"; 
    fastgltf::FileExporter exporter;
    exporter.setImagePath("textures");
    exporter.setBufferPath("datapath");
    fastgltf::ExportOptions options = fastgltf::ExportOptions::None;

    auto result = exporter.writeGltfJson(asset,path / "scene.gltf");
    
    if(result == fastgltf::Error::None){

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        Utils::Logger::LogInfoClient("Scene saved in " + std::to_string(duration.count()) + "ms");
        Utils::Logger::LogSuccess("Scene saved to " + path.string());
        return;
    }else{
        Utils::Logger::LogErrorClient("Failed to save the scene:" + std::string(fastgltf::getErrorMessage(result)));
    }

    Clear();

    Utils::Logger::LogSuccessClient("Scene saved to " + path.string());
}

void ApplicationCore::GLTFExporter::ParseBuffers(fastgltf::Asset &asset, AssetsManager& assetsManager)
{
    //========================================
    // CREATE BUFFERS 
    //========================================
    fastgltf::Buffer m_vertexBuffer;
    fastgltf::Buffer m_indexBuffer;
    //fastgltf::Buffer m_textureBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    //std::vector<std::byte> imageData;

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackVertexBuffer()){
        vertices.insert(vertices.end(), buffer.data.begin(), buffer.data.end());        
    }

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackIndexBuffers()){
        indices.insert(indices.end(), buffer.data.begin(), buffer.data.end());        
    }

    m_vertexBuffer.byteLength = vertices.size() * sizeof(Vertex);
    std::vector<std::byte> vertexBufferVector(reinterpret_cast<std::byte*>(vertices.data()), reinterpret_cast<std::byte*>(vertices.data()) + m_vertexBuffer.byteLength);
    if (reinterpret_cast<uintptr_t>(vertices.data()) % alignof(float) != 0) {
        Utils::Logger::LogErrorClient("Vertex buffer is not aligned to float");
    }
    m_vertexBuffer.data  = fastgltf::sources::Vector{vertexBufferVector};
    m_vertexBuffer.name = "Vertex buffers";

    m_indexBuffer.byteLength = indices.size() * sizeof(uint32_t);
    std::vector<std::byte> indexBufferVector(reinterpret_cast<std::byte*>(indices.data()), reinterpret_cast<std::byte*>(indices.data()) + m_indexBuffer.byteLength);
    m_indexBuffer.name = "Index buffers";
    m_indexBuffer.data = fastgltf::sources::Vector{indexBufferVector};

    //============================================
    // STORE VERTEX BUFFERS TO THE ASSET
    //============================================
    asset.buffers.resize(2);
    asset.buffers[0] = std::move(m_vertexBuffer);
    asset.buffers[1] = std::move(m_indexBuffer);
    //asset.buffers[2] = std::move(m_textureBuffer);
}

void ApplicationCore::GLTFExporter::ParseTexture(fastgltf::Asset &asset, AssetsManager &assetsManager)
{
    std::vector<std::byte> imageData;
    m_fetchedTextureViews = std::move(assetsManager.ReadBackAllTextures(imageData));

    for(auto& texture : m_fetchedTextureViews){
        fastgltf::Image image;

        
        std::filesystem::path imgPath = "textures/" + texture.path.substr(texture.path.rfind("/") + 1);
        fastgltf::URI imagepath(imgPath);
        image.data = fastgltf::sources::URI{.uri =imagepath};
        image.name = texture.path;
        asset.images.push_back(std::move(image));

        fastgltf::Texture t;
        t.imageIndex = asset.images.size() - 1;   
        asset.textures.push_back(std::move(t)); 

        m_textureToIndex[texture.textureAsset] = asset.textures.size() - 1;

    }

    Utils::Logger::LogSuccessClient("Textures parsed successfuly");
}

void ApplicationCore::GLTFExporter::ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager &assetsManager, fastgltf::Asset &asset)
{
    
    //===================================================
    // PARSE MESH DATA
    //===================================================
    if(IsNodeValid(sceneNode))
    {
        fastgltf::Node node{};
        if(sceneNode->HasMesh()){
            ParseMesh(asset, sceneNode->GetMesh());
            node.meshIndex = m_meshToIndex[sceneNode->GetMesh()];
        }else{
            node.meshIndex = std::nullopt;
        }
        
        node.name = sceneNode->GetName();
    
        fastgltf::TRS  trs;

        auto& quat =  sceneNode->m_transformation->GetRotationsQuat();
        fastgltf::math::quat<float> fastgltf_Quat;
        fastgltf_Quat.x() = quat.x;
        fastgltf_Quat.y() = quat.y;
        fastgltf_Quat.z() = quat.z;
        fastgltf_Quat.w() = quat.w;

        trs.translation = fastgltf::math::vec<float, 3>(sceneNode->m_transformation->GetPosition().x, sceneNode->m_transformation->GetPosition().y, sceneNode->m_transformation->GetPosition().z);
        trs.rotation = fastgltf_Quat;
        trs.scale = fastgltf::math::vec<float, 3>(sceneNode->m_transformation->GetScale().x, sceneNode->m_transformation->GetScale().y, sceneNode->m_transformation->GetScale().z);

        node.transform = trs;
        asset.nodes.push_back(std::move(node));
        
        m_nodes[sceneNode] = asset.nodes.size() - 1; 
        
    }

    for (auto& child : sceneNode->GetChildrenByRef())
    {
        ParseScene(child, assetsManager, asset);
    }
}

void ApplicationCore::GLTFExporter::ParseMaterial(fastgltf::Asset &asset, AssetsManager &assetsManager)
{
 

    for (auto & mat : assetsManager.GetAllMaterials())
    {
        fastgltf::Material material;
        auto& matValues = mat->GetMaterialDescription().values;
    
        material.name = mat->GetMaterialName();
        material.alphaMode = mat->IsTransparent() ? fastgltf::AlphaMode::Blend : fastgltf::AlphaMode::Opaque;  
        material.pbrData.baseColorFactor = fastgltf::math::vec<float, 4>(matValues.diffuse.x, matValues.diffuse.y, matValues.diffuse.z, matValues.diffuse.w);
        material.pbrData.metallicFactor = matValues.metalness;
        material.pbrData.roughnessFactor = matValues.roughness;
        material.emissiveFactor.x() = matValues.emissive_strength.x;
        material.emissiveFactor.y() = matValues.emissive_strength.y;
        material.emissiveFactor.z() = matValues.emissive_strength.z;
        material.emissiveStrength = matValues.emissive_strength.w;

        auto &materialPaths = mat->GetMaterialPaths();
        if(!materialPaths.NormalMapPath.empty()){
            fastgltf::NormalTextureInfo normalTextureInfo;
            normalTextureInfo.textureIndex = m_textureToIndex[mat->GetTextureRawPtr(ETextureType::normal)];
            material.normalTexture = std::move(normalTextureInfo); 
        }
        
        if(!materialPaths.DiffuseMapPath.empty()){
            fastgltf::TextureInfo diffuseTextureInfo;
            diffuseTextureInfo.textureIndex = m_textureToIndex[mat->GetTextureRawPtr(ETextureType::Diffues)];
            material.pbrData.baseColorTexture = std::move(diffuseTextureInfo);
        }
        
        if(!materialPaths.ArmMapPath.empty()){
            fastgltf::TextureInfo armTextureInfo;
            armTextureInfo.textureIndex = m_textureToIndex[mat->GetTextureRawPtr(ETextureType::arm)];
            material.pbrData.metallicRoughnessTexture = std::move(armTextureInfo);
        }
        if(!materialPaths.EmmisivePath.empty()){
            fastgltf::TextureInfo emissiveTextureInfo;
            emissiveTextureInfo.textureIndex = m_textureToIndex[mat->GetTextureRawPtr(ETextureType::Emissive)];
            material.emissiveTexture = std::move(emissiveTextureInfo);
        }
    
        
        asset.materials.push_back(std::move(material));
        m_materialToIndex[mat] = asset.materials.size() - 1;
    }
    
}

void ApplicationCore::GLTFExporter::ParseMesh(fastgltf::Asset &asset, std::shared_ptr<StaticMesh> mesh)
{
        //auto& mesh = mesh;
    
        //============================================
        // CREATE BUFFER VIEWS FOR VERTEX   
        //============================================
        fastgltf::BufferView vertexBufferView;
        vertexBufferView.bufferIndex = 0;
        vertexBufferView.byteLength = mesh->GetMeshData()->vertexData.size;
        vertexBufferView.byteOffset = mesh->GetMeshData()->vertexData.offset;
        vertexBufferView.byteStride = sizeof(Vertex);
        vertexBufferView.name = "Vertex buffer view";
        vertexBufferView.target = fastgltf::BufferTarget::ArrayBuffer;
        asset.bufferViews.push_back(std::move(vertexBufferView));

        //============================================
        // CREATE BUFFER VIEWS FOR INDEX  
        //============================================
        fastgltf::BufferView indexBufferView;
        indexBufferView.bufferIndex = 1;
        indexBufferView.byteLength =  mesh->GetMeshData()->indexData.size;
        indexBufferView.byteOffset =  mesh->GetMeshData()->indexData.offset;
        indexBufferView.name = "Index buffer view";
        indexBufferView.target = fastgltf::BufferTarget::ElementArrayBuffer;
        asset.bufferViews.push_back(std::move(indexBufferView));

        //============================================
        // CREATE POSITION ACCESSOR
        //============================================
        fastgltf::Accessor positionAccessor;
        positionAccessor.bufferViewIndex = asset.bufferViews.size()-2;
        positionAccessor.byteOffset = offsetof(Vertex, position);
        positionAccessor.componentType = fastgltf::ComponentType::Float;
        positionAccessor.count = mesh->GetMeshData()->vertexData.size /sizeof(Vertex);
        positionAccessor.type = fastgltf::AccessorType::Vec3;
        positionAccessor.name = "Position accessor";

        std::pmr::vector<double>min{(double)mesh->GetMeshData()->bounds.min.x, (double)mesh->GetMeshData()->bounds.min.y, (double)mesh->GetMeshData()->bounds.min.z};
        std::pmr::vector<double>max{(double)mesh->GetMeshData()->bounds.max.x, (double)mesh->GetMeshData()->bounds.max.y, (double)mesh->GetMeshData()->bounds.max.z};

        positionAccessor.min = std::move(min);
        positionAccessor.max = std::move(max);
        asset.accessors.push_back(std::move(positionAccessor));
    

        //============================================
        // CREATE NORMAL ACCESSOR
        //============================================
        fastgltf::Accessor normalAccessor;
        normalAccessor.bufferViewIndex = asset.bufferViews.size() -2;
        normalAccessor.byteOffset = offsetof(Vertex, normal);
        normalAccessor.componentType = fastgltf::ComponentType::Float;
        normalAccessor.count = mesh->GetMeshData()->vertexData.size / sizeof(Vertex);
        normalAccessor.name = "Normal accessor";
        normalAccessor.type = fastgltf::AccessorType::Vec3;
        asset.accessors.push_back(std::move(normalAccessor));


        //============================================
        // UV ACCESSOR
        //============================================
        fastgltf::Accessor uvAccessor;
        uvAccessor.bufferViewIndex = asset.bufferViews.size() -2; // indices are the latest - 1  buffer pushed to this vector
        uvAccessor.byteOffset = offsetof(Vertex, uv);
        uvAccessor.componentType = fastgltf::ComponentType::Float;
        uvAccessor.count = mesh->GetMeshData()->vertexData.size / sizeof(Vertex);
        uvAccessor.name = "UV accessor";
        uvAccessor.type = fastgltf::AccessorType::Vec2;
        asset.accessors.push_back(std::move(uvAccessor));

        //============================================
        // IDICES ACCESSOR
        //============================================
        fastgltf::Accessor indicesAccessor;
        indicesAccessor.bufferViewIndex = asset.bufferViews.size()- 1; // indices is the latest buffer pushed to this vector
        indicesAccessor.byteOffset = 0;
        indicesAccessor.componentType = fastgltf::ComponentType::UnsignedInt;
        indicesAccessor.count = mesh->GetMeshData()->indexData.size / sizeof(uint32_t);
        indicesAccessor.name = "Indices accessor";
        indicesAccessor.type = fastgltf::AccessorType::Scalar;
        asset.accessors.push_back(std::move(indicesAccessor));


        fastgltf::Mesh m;
        fastgltf::Primitive primitive;
        primitive.materialIndex = m_materialToIndex[mesh->GetMaterial()];
        primitive.attributes = {
            {"POSITION", asset.accessors.size()-4},
            {"NORMAL", asset.accessors.size()-3},
            {"TEXCOORD_0", asset.accessors.size()-2}
        };
        primitive.indicesAccessor = asset.accessors.size() - 1;
        
        m.name = mesh->GetName();
        m.primitives.push_back(std::move(primitive));
        asset.meshes.push_back(std::move(m));


        m_meshToIndex[mesh] =asset.meshes.size() - 1;
}

void ApplicationCore::GLTFExporter::OrganiseScene(fastgltf::Asset &asset)
{
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {   
       if(!it->first->GetChildrenByRef().empty()){
            for(auto& child : it->first->GetChildrenByRef()){
                asset.nodes[it->second].children.push_back(m_nodes[child]);
            }
       }
    }    
}

void ApplicationCore::GLTFExporter::CreateScene(fastgltf::Asset &asset,Scene& scene)
{
    asset.scenes.resize(1);
    asset.scenes[0].name = "Vulkan-RTX-saved-scene";
    // node at index 0 is allways the root
    for (auto& rootChild : scene.GetRootNode()->GetChildrenByRef())
    {
        if (IsNodeValid(rootChild))
            asset.scenes[0].nodeIndices.push_back(m_nodes[rootChild]);
    }
    if (asset.scenes[0].nodeIndices.empty())
    {
        // in case there are no children use first node to pack the scene
        asset.scenes[0].nodeIndices.push_back(0);
    }
    
}

void ApplicationCore::GLTFExporter::ParseLights(Scene& scene)
{
    auto& lightInfo  = scene.GetSceneLightInfo();

    mINI::INIFile iniFile(m_lightInfoPath);
    mINI::INIStructure ini;

    //=======================================
    // POINT LIGHTS
    //========================================
    ini["point"]["count"] = std::to_string(lightInfo.PointLightInfos.size());
    for (int i = 0; i<lightInfo.PointLightInfos.size(); i++)
    {
        auto pointLightKey = "point-light-" + std::to_string(i);
        auto& pointLight = lightInfo.PointLightInfos[i];
        if (pointLight != nullptr )
        {
            if (pointLight->isPointLightInUse)
            {
                ini[pointLightKey].set({
                    {"colR" , std::to_string(pointLight->colour.x)},
                    {"colG" , std::to_string(pointLight->colour.y)},
                    {"colB" , std::to_string(pointLight->colour.z)},
                    {"colA" , std::to_string(pointLight->colour.w)},

                    {"posX", std::to_string(pointLight->position.x)},
                    {"posY", std::to_string(pointLight->position.y)},
                    {"posZ", std::to_string(pointLight->position.z)},

                    {"constantFactor", std::to_string(pointLight->constantFactor)},
                    {"linearFactor", std::to_string(pointLight->linearFactor)},
                    {"quadraticFactor", std::to_string(pointLight->quadraticFactor)},

                    {"isInUse", std::to_string(pointLight->isPointLightInUse)}
                });
            }
        }
    }

    //=======================================
    // AREA LIGHTS
    //========================================
    ini["area"]["count"] = std::to_string(lightInfo.AreaLightInfos.size());
    for (int i = 0; i<lightInfo.AreaLightInfos.size(); i++)
    {
        auto areaLightKey = "area-light-" + std::to_string(i);
        auto& areaLight = lightInfo.AreaLightInfos[i];
        if (areaLight != nullptr)
        {
            ini[areaLightKey].set({
                {"colR" , std::to_string(areaLight->colour.x)},
                {"colG" , std::to_string(areaLight->colour.y)},
                {"colB" , std::to_string(areaLight->colour.z)},
                {"colA" , std::to_string(areaLight->intensity)},

                {"posX", std::to_string(areaLight->position.x)},
                {"posY", std::to_string(areaLight->position.y)},
                {"posZ", std::to_string(areaLight->position.z)},

                {"scaleX", std::to_string(areaLight->scale.x)},
                {"scaleY", std::to_string(areaLight->scale.y)},

                {"quatX", std::to_string(areaLight->orientation.x)},
                {"quatY", std::to_string(areaLight->orientation.y)},
                {"quatZ", std::to_string(areaLight->orientation.z)},
                {"quatW", std::to_string(areaLight->orientation.w)},


                {"twoSided", std::to_string(areaLight->twoSided)},
                {"isInUse", std::to_string(areaLight->isAreaLightInUse)}

            });
        }
    }

    // save directional light

    if (lightInfo.DirectionalLightInfo != nullptr)
    {
        auto& dirLight = lightInfo.DirectionalLightInfo;
        ini["directional"]["directionX"] = std::to_string(dirLight->direction.x);
        ini["directional"]["directionY"] = std::to_string(dirLight->direction.y);
        ini["directional"]["directionZ"] = std::to_string(dirLight->direction.z);

        ini["directional"]["colR"] = std::to_string(dirLight->colour.x);
        ini["directional"]["colG"] = std::to_string(dirLight->colour.y);
        ini["directional"]["colB"] = std::to_string(dirLight->colour.z);
        ini["directional"]["colA"] = std::to_string(dirLight->colour.w); // intensity
    }



    assert(iniFile.generate(ini, true) == true && "Failed to save light info)");

}

void ApplicationCore::GLTFExporter::Clear()
{
    m_materialToIndex.clear();
    m_meshToIndex.clear();
    m_nodes.clear();
    m_childNodes.clear();
    m_textureToIndex.clear();
    
    m_nodeCounter = 0;  
}

bool ApplicationCore::GLTFExporter::IsNodeValid(const std::shared_ptr<SceneNode>& sceneNode)
{
    return (sceneNode->GetSceneNodeMetaData().nodeType != ENodeType::AreaLightNode &&
          sceneNode->GetSceneNodeMetaData().nodeType != ENodeType::DirectionalLightNode &&
          sceneNode->GetSceneNodeMetaData().nodeType != ENodeType::PointLightNode &&
          sceneNode->GetSceneNodeMetaData().nodeType != ENodeType::SkyBoxNode &&
          sceneNode->GetName() != "Root-Node");

}
