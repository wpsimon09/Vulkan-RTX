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

        auto& openProjectPath   = inputParser.getCmdOption("--open");
        auto& createProjectPath = inputParser.getCmdOption("--create");
        bool  printInfo         = inputParser.cmdOptionExists("-h") || inputParser.cmdOptionExists("--help");

        if(printInfo)
        {
            project.PrintHelp();
            return 0;
        }
        if(!openProjectPath.empty())
        {
            project.OpenFrom(openProjectPath);
        }
        else if(!createProjectPath.empty())
        {
            project.CrateNew(createProjectPath);
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
