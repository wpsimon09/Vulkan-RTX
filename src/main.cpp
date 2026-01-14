//
// Created by wpsimon09 on 16/09/24.
//

#include <iostream>
#include "VulkanRtx.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Project/Project.hpp"
#include "Application/Utils/InputParser.hpp"


int main(int argc, char* argv[])
{
    try
    {
        //==============================================
        // Parse the cmd options
        CLI::InputParser         inputParser(argc, argv);
        ApplicationCore::Project project;

        auto& openProjectPath   = inputParser.getCmdOption("--open", "-o");
        auto& createProjectPath = inputParser.getCmdOption("--create", "-c");
        auto& newProjectName    = inputParser.getCmdOption("--name", "-n");
        bool  printInfo         = inputParser.cmdOptionExists("-h") || inputParser.cmdOptionExists("--help");

        if(printInfo)
        {
            project.PrintHelp();
            return 0;
        }
        if(!openProjectPath.empty())
        {
            project.OpenFrom(openProjectPath);
            return 0;
        }
        else if(!createProjectPath.empty())
        {
            project.CrateNew(createProjectPath, newProjectName);
            return 0;
        }


        Application application(project);
        Utils::Logger::LogSuccess("Starting the application...");
        application.Run();
        Utils::Logger::LogInfo("Application is stopping...");
    }
    catch(std::exception& e)
    {
        Utils::Logger::LogError(e.what());
    }
    return 0;
}
