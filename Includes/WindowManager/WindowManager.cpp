//
// Created by wpsimon09 on 17/09/24.
//

#include "WindowManager.h"

WindowManager::WindowManager()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(800, 600, "Hello World!", NULL, NULL);
}

bool WindowManager::InitWindow()
{
}

const float& WindowManager::getWindowWidth() const
{
}

const float& WindowManager::getWindowHeight() const
{
}

const glm::vec2& WindowManager::getWindowDimensions()
{
}

WindowManager::~WindowManager()
{
}
