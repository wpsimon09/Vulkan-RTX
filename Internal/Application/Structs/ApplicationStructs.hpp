//
// Created by wpsimon09 on 26/10/24.
//
#pragma once
#ifndef APPLICATIONSTRUCTS_HPP
#define APPLICATIONSTRUCTS_HPP
#include <iostream>
#include <ostream>

struct CameraUpdateInfo
{
    float RotatePolarValue = 0.0f;
    float RotateAzimuthValue = 0.0f;
    float ZoomValue = 0.0f;
    float NewWidth = 0.0f;
    float NewHeight = 0.0f;
    float MoveX = 0.0f;
    float MoveY = 0.0f;
    float MoveNear = 0.0f;

    void Reset() {
        RotatePolarValue = 0;
        RotateAzimuthValue = 0;
        ZoomValue = 0;
        NewWidth = 0;
        NewHeight = 0;
        MoveX = 0;
        MoveY = 0;
        MoveNear = 0;
    }

    void Print() const {
        /**
        std::cout << "===================================================="<<std::endl;
        std::cout << "RotatePolarValue: " << RotatePolarValue << std::endl;
        std::cout << "RotateAzimuthValue: " << RotateAzimuthValue << std::endl;
        std::cout << "ZoomValue: " << ZoomValue << std::endl;
        std::cout << "MoveX: " << MoveX << std::endl;
        std::cout << "MoveY: " << MoveY << std::endl;
        std::cout << "===================================================="<<std::endl;
         */
    }
};

struct LightUpdateInfo
{
    float moveX = 0.0f;
    float moveY = 0.0f;
    float moveZ = 0.0f;
    void Reset()
    {
        moveX = 0.0f;
        moveY = 0.0f;
        moveZ = 0.0f;
    }
    void Print() const {

        std::cout << "===================================================="<<std::endl;
        std::cout << "Movex: " << moveX << std::endl;
        std::cout << "MoveY: " << moveY << std::endl;
        std::cout << "===================================================="<<std::endl;

    }
};

#endif //APPLICATIONSTRUCTS_HPP
