//
// Created by wpsimon09 on 26/10/24.
//
#pragma once
#ifndef APPLICATIONSTRUCTS_HPP
#define APPLICATIONSTRUCTS_HPP

struct CameraUpdateInfo
{
    float RotatePolarValue = 0.0f;
    float RotateAzimuthValue = 0.0f;
    float ZoomValue = 0.0f;
    float NewWidth = 0.0f;
    float NewHeight = 0.0f;
    float MoveX = 0.0f;
    float MoveY = 0.0f;

    void Reset() {
        RotatePolarValue = 0;
        RotateAzimuthValue = 0;
        ZoomValue = 0;
        NewWidth = 0;
        NewHeight = 0;
        MoveX = 0;
        MoveY = 0;
    }
};

#endif //APPLICATIONSTRUCTS_HPP
