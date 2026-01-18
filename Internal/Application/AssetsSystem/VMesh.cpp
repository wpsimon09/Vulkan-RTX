//
// Created by simon on 18/01/2026.
//

#include "VMesh.hpp"

#include <fstream>

namespace ApplicationCore {
VMesh::VMesh()
    : VAsset2<ApplicationCore::VMeshHeader>(".Vmesh")
{
}

VMesh::VMesh(std::string name, std::filesystem::path& directory, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    : VAsset2<ApplicationCore::VMeshHeader>(".Vmesh")
{
    //==================================
    // save to file and delete the object
    m_fileHeader.uuid        = uuid::generate_uuid_v4();
    m_fileHeader.name        = name;
    m_fileHeader.indexCount  = indices.size();
    m_fileHeader.vertexCount = vertices.size();
    m_path                   = directory / (name + m_fileType);

    std::ofstream file(m_path, std::ios::binary);

    if(file.is_open())
    {
        file.write(reinterpret_cast<char*>(&m_fileHeader), sizeof(m_fileHeader));

        // Write vertex data
        file.write(reinterpret_cast<const char*>(vertices.data()), sizeof(Vertex) * vertices.size());
        // Write index data
        file.write(reinterpret_cast<const char*>(indices.data()), sizeof(uint32_t) * indices.size());

        file.close();
    }
}
bool     VMesh::Save(std::filesystem::path& path) {}
bool     VMesh::Load() {}
std::any VMesh::LoadData() {}
}  // namespace ApplicationCore