#ifndef SCENEDATA_HPP
#define SCENEDATA_HPP

#include <memory>
#include <vector>


struct FogVolumeParameters;
struct PBRMaterialDescription;

namespace ApplicationCore {
class VTextureAsset;
class SceneNode;
class StaticMesh;

/**
  * Struct that contains flat lists of scene data used to indexing in shaders and buffers
  */
struct SceneData
{
    std::vector<std::shared_ptr<ApplicationCore::StaticMesh>> meshes;
    std::vector<PBRMaterialDescription*>                      pbrMaterials;
    std::vector<std::shared_ptr<SceneNode>>                   nodes;
    std::vector<std::shared_ptr<VTextureAsset>>               textures;

    void AddEntry(std::shared_ptr<ApplicationCore::SceneNode>& node);
    bool CheckIndexValidity(size_t arraySize, size_t index);
    void RemoveEntry(const ApplicationCore::SceneNode& node);

    void Reset();

  private:
    void IndexNode(std::shared_ptr<ApplicationCore::SceneNode>& node);
};
}  // namespace ApplicationCore

#endif
