//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VMESH_HPP
#define VULKAN_RTX_VMESH_HPP
#include "FileHeaders.hpp"
#include "VAsset2.hpp"

namespace ApplicationCore {


class VMesh : public VAsset2<VMeshHeader, VulkanStructs::VMeshData2>
{
  public:
    VMesh();
    /**
     * Loads already existing mesh from the file
     * @param databaseEntry database entry specified in .VMesh
     */
    VMesh(AssetEntry& databaseEntry);

    /**
   * Loads the mesh from the .VMesh file
   * @param databaseEntry databse entry requested from VAsset.json
   * @param materialUUID material of the mesh
   * @param vertices vertices buffer
   * @param indices indices buffer 
   */
    VMesh(AssetEntry& databaseEntry, uuid::UUID materialUUID, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    bool     Save(std::filesystem::path& path) override;
    bool     Load() override;
    std::any LoadData() override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VMESH_HPP
