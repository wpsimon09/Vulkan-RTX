//
// Created by wpsimon09 on 02/03/25.
//

#ifndef LOADSCENELIGHTS_HPP
#define LOADSCENELIGHTS_HPP
#include <filesystem>
namespace ApplicationCore
{
    class Scene;

    void LoadSceneLights(ApplicationCore::Scene& scene, std::filesystem::path& lightInfoPath);
}

#endif //LOADSCENELIGHTS_HPP
