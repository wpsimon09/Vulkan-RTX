//
// Created by wpsimon09 on 22/09/24.
//

#ifndef WINDOWMANAGER_HPP
#define WINDOWMANAGER_HPP

#define GLFW_INCLUDE_VULKAN;
#include <glm/vec2.hpp>
class GLFWwindow;
struct CameraMovement;

class WindowManager {
public:
    WindowManager(int windowWidth, int windowHeight);

    void InitWindow();

    static int GetWindowWidth() ;
    static int GetWindowHeight();
    GLFWwindow* GetWindow() const;

    static void GetRequiredExtensions(const char**& extensions, uint32_t& count );

    ~WindowManager();
private:
    static void MousePositionCallback(GLFWwindow *window, double xpos, double ypos);
    static void MouseClickCallback(GLFWwindow *window, int button, int action, int mods);
    static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
private:
    static inline bool m_isFirstMouse = false;
    static inline CameraMovement m_cameraMovement;
    static inline GLFWwindow* m_window;
    static inline int m_width, m_height;
    static inline bool m_isMousePressed;
    static inline float m_lastX = 0.0f, m_lastY = 0.0f;
    static inline glm::vec2 m_mousePos;
};


#endif //WINDOWMANAGER_HPP
