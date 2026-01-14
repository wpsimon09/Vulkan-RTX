//
// Created by simon on 14/01/2026.
//

#include "Project.hpp"

#include "Application/Logger/Logger.hpp"


namespace ApplicationCore {
Project::Project() {}

void Project::PrintHelp()
{
    std::cout << "=====================================\n";
    std::cout << "-h or --help : print this help message\n";
    std::cout << "-c or --create [path] :creates new project\n";
    std::cout << "-o or --open [path]/VProject.json: print this help message\n";
}
void Project::CrateNew(std::filesystem::path path)
{
    Utils::Logger::LogInfoCLI("Creating new project at path:    " + path.string());
    m_projectPath = path;
}

void Project::OpenFrom(std::filesystem::path path)
{
    Utils::Logger::LogInfoCLI("Opening project from path :" + path.string());
    m_projectPath = path;
}
}  // namespace ApplicationCore