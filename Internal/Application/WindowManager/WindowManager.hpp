//
// Created by wpsimon09 on 22/09/24.
//

#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#define GLFW_INCLUDE_VULKAN
#include <glm/vec2.hpp>

#include "Application/Structs/ApplicationStructs.hpp"
class GLFWwindow;
struct CameraUpdateInfo;

class WindowManager {
public:
    WindowManager(int windowWidth, int windowHeight);

    void InitWindow();
    bool GetIsDirty() const { return m_isDirty; }
    ClientUpdateInfo& GetLightMovement()   { m_isDirty = false;  return m_clientUpdate;    };
    CameraUpdateInfo& GetCameraMovement() { m_isDirty = false ; return m_cameraMovement; };

    static int GetWindowWidth() ;
    static int GetWindowHeight();
    GLFWwindow* GetWindow() const;

    static void GetRequiredExtensions(const char**& extensions, uint32_t& count );

    ~WindowManager();
private:
    static void MousePositionCallback(GLFWwindow *window, double xpos, double ypos);
    static void MouseClickCallback(GLFWwindow *window, int button, int action, int mods);
    static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void FrameBufferResizeCallback(GLFWwindow *window, int width, int height);
    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
private:
    bool m_isFirstMouse = false;
    CameraUpdateInfo m_cameraMovement;
    ClientUpdateInfo m_clientUpdate;
    static inline GLFWwindow* m_window;
    int m_width, m_height;
    bool m_isMousePressed;
    bool m_isShiftPressed;
    float m_lastX = 0.0f, m_lastY = 0.0f;
    bool m_isDirty = false;
    glm::vec2 m_mousePos;
};


#endif //WINDOWMANAGER_HPP
