//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_PROJECT_HPP
#define VULKAN_RTX_PROJECT_HPP
#include "json.hpp"


#include <filesystem>

namespace ApplicationCore {

using json = nlohmann::json;

class Project
{
  public:
    Project();
    void PrintHelp();
    void CrateNew(std::filesystem::path path);
    void OpenFrom(std::filesystem::path path);

  private:
    std::filesystem::path m_projectPath;
    json                  m_json;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_PROJECT_HPP
