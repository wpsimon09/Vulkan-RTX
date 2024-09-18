//
// Created by wpsimon09 on 16/09/24.
//

#include <iostream>

#include "Application.h"

int main()
{
    Application application;
    try
    {
        application.Run();
    }catch(std::exception& e)
    {
        std::cerr<<"============================= *Fatal error occured* ============================================="<<std::endl;
        std::cerr<<e.what()<<std::endl;
        std::cerr<<"================================================================================================="<<std::endl;
    }
    return 0;
}
