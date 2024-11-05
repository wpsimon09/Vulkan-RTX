//
// Created by wpsimon09 on 26/10/24.
//
#pragma once
#ifndef APPLICATIONSTRUCTS_HPP
#define APPLICATIONSTRUCTS_HPP

struct CameraUpdateInfo
{
    float RotatePolarValue;
    float RotateAzimuthValue;
    float ZoomValue;
    float NewWidth;
    float NewHeight;

    void Reset() {
        RotatePolarValue = 0;
        RotateAzimuthValue = 0;
        ZoomValue = 0;
        NewWidth = 0;
        NewHeight = 0;
    }
};

#endif //APPLICATIONSTRUCTS_HPP
