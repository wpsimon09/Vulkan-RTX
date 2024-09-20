//
// Created by wpsimon09 on 16/09/24.
//

#include <iostream>

import Application;
import Logger;
int main()
{
    Application application;
    try
    {
        application.Run();
    }catch(std::exception& e)
    {
        Logger::LogError(e.what());
    }
    return 0;
}
