//
// Created by wpsimon09 on 22/09/24.
//

#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <bits/ios_base.h>

#include "Application/Structs/ApplicationStructs.hpp"

WindowManager::WindowManager(int windowWidth, int windowHeight):m_cameraMovement{0.0f, 0.0f, 0.0f}
{
    m_width = windowWidth;
    m_height = windowHeight;
    m_isDirty = false;
}

void WindowManager::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan-RTX", NULL, NULL);
    glfwSetWindowUserPointer(m_window, this);

    //Call backs
    glfwSetFramebufferSizeCallback(m_window, FrameBufferResizeCallback);
    glfwSetCursorPosCallback(m_window, MousePositionCallback);
    glfwSetMouseButtonCallback(m_window, MouseClickCallback);
    glfwSetScrollCallback(m_window, MouseScrollCallback);
}

int WindowManager::GetWindowWidth() {
    auto winm = reinterpret_cast<WindowManager*>(m_window);
    glfwGetFramebufferSize(m_window, &winm->m_width, &winm->m_height);
    return winm->m_width;
}

int WindowManager::GetWindowHeight() {
    auto winm = reinterpret_cast<WindowManager*>(m_window);
    glfwGetFramebufferSize(m_window, &winm->m_width, &winm->m_height);
    return winm->m_height;
}

GLFWwindow* WindowManager::GetWindow()const
{
    return m_window;
}

void WindowManager::GetRequiredExtensions(const char**& extensions, uint32_t& count)
{
    extensions = glfwGetRequiredInstanceExtensions(&count);
}


WindowManager::~WindowManager()
{
    glfwDestroyWindow(m_window);
}

void WindowManager::MousePositionCallback(GLFWwindow *window, double xpos, double ypos) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(m_window));
    winm->m_mousePos.x = static_cast<float>(xpos);
    winm->m_mousePos.y = static_cast<float>(ypos);

    auto pointerX = (float)xpos;
    auto pointerY = (float)ypos;
    if (winm->m_isFirstMouse)
    {
        winm->m_lastX = xpos;
        winm->m_lastY = ypos;
        winm->m_isFirstMouse = false;;
    }

    float xOffset = xpos - winm->m_lastX;
    float yOffset = winm->m_lastY - ypos; // Invert the sign here

    winm->m_lastX = xpos;
    winm->m_lastY = ypos;

    xOffset *= 0.01;
    yOffset *= 0.01;

    if (xOffset != 0.0 && winm->m_isMousePressed)
    {
        // have isDirty = true here and in the getter set dirty to false so that cmare is not calculating new stuff every frameP
        // in camera update send this to the camera to know how much to rotate, if nothing happend than dont send it
        winm->m_cameraMovement.RotateAzimuthValue =  xOffset;
        winm->m_isDirty = true;
    }

    if (yOffset != 0.0 && winm->m_isMousePressed)
    {
        winm->m_cameraMovement.RotatePolarValue = -yOffset;
        winm->m_isDirty = true;
    }
}

void WindowManager::MouseClickCallback(GLFWwindow *window, int button, int action, int mods) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    GLFWcursor* hand = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CURSOR_NORMAL);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            winm->m_isMousePressed = true;
            glfwSetCursor(winm->m_window, hand);
        }
        else if (action == GLFW_RELEASE)
        {
            winm->m_isMousePressed = false;
            glfwSetCursor(winm->m_window, cursor);
        }
    }

}

void WindowManager::MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer((window)));
    winm->m_cameraMovement.ZoomValue = (float)yoffset;
}

void WindowManager::FrameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto winm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    winm->m_width = width;
    winm->m_height = height;
    winm->m_cameraMovement.NewHeight = static_cast<float>(height);
    winm->m_cameraMovement.NewWidth = static_cast<float>(width);
}
