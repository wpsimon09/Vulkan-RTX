//
// Created by wpsimon09 on 26/10/24.
//
#pragma once
#ifndef APPLICATIONSTRUCTS_HPP
#define APPLICATIONSTRUCTS_HPP
#include <iostream>
#include <ostream>

#include "Application/Enums/ClientEnums.hpp"
#include "glm/glm.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

struct CameraUpdateInfo
{
    float RotatePolarValue = 0.0f;
    float RotateAzimuthValue = 0.0f;
    float ZoomValue = 0.0f;
    float NewWidth = 0.0f;
    float NewHeight = 0.0f;
    float MoveX = 0.0f;
    float MoveY = 0.0f;
    float MoveZ = 0.0f;
    float MoveNear = 0.0f;

    void Reset() {
        RotatePolarValue = 0;
        RotateAzimuthValue = 0;
        ZoomValue = 0;
        NewWidth = 0;
        NewHeight = 0;
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

struct ClientUpdateInfo
{
    float moveLightX = 0.0f;
    float moveLightY = 0.0f;
    float moveLightZ = 0.0f;
    bool isRTXon = false;
    void Reset()
    {
        moveLightX = 0.0f;
        moveLightY = 0.0f;
        moveLightZ = 0.0f;
    }
    void Print() const {

        /*
        std::cout << "===================================================="<<std::endl;
        std::cout << "Movex: " << moveLightX << std::endl;
        std::cout << "MoveY: " << moveLightY << std::endl;
        std::cout << "===================================================="<<std::endl;
        */
    }
};

struct TextureBufferInfo
{
    std::byte* data;
    size_t size;
    std::string textureID;
};

struct SceneStatistics
{
    int sceneNumberOfTriangles = 0;
    int numberOfMeshes = 0;
    void Reset()
    {
        numberOfMeshes = 0;
    }
};

struct Ray
{

    glm::vec3 origin;
    glm::vec3 direction;
    float length;
};


struct SceneNodeMetaData
{
    bool IsParentNode = false;
    bool HasMesh = false;
    bool IsVisible = true;
    bool ShowInEditor = true;
    bool IsSelected = false;
    bool IsSelectedFromWorld = false;
    bool IsAnyChildSelected = false;
    bool IsOpen = false;
    bool VisibleInRayTracing = false;
    bool CastsShadows = false;
    bool FrustumCull = true;
    ENodeType nodeType = ENodeType::Node;
    int ID = 0;
    int exportID = 0;
};


#endif //APPLICATIONSTRUCTS_HPP
