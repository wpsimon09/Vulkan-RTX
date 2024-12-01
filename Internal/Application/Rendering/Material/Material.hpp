//
// Created by wpsimon09 on 26/11/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <memory>
#include <string>
#include <vector>



namespace VulkanCore
{
    class VImage;
}

namespace ApplicationCore {

class Material {

    struct MaterialPaths
    {
        std::string DiffuseMapPath;
        std::string ArmMapPath;
        std::string NormalMapPath;
    };

public:
    Material();

    void Sync();
private:
    std::array<std::shared_ptr<VulkanCore::VImage>,3> m_textures;


};

} // ApplicationCore

#endif //MATERIAL_HPP
