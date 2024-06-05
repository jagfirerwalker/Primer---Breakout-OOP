#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>


class Push {
public:
    Push();
    ~Push();

    void setupScreen(int virtualWidth, int virtualHeight, int windowWidth, int windowHeight, bool vsync = true, bool fullscreen = false, bool resizable = false);
    void start();
    void finish();

    void resize(int width, int height);

    int getWidth() const;
    int getHeight() const;

    GLFWwindow* getWindow() const;

private:
    int m_virtualWidth;
    int m_virtualHeight;
    int m_windowWidth;
    int m_windowHeight;
    bool m_fullscreen;
    bool m_resizable;
    GLFWwindow* m_window;

    float m_scaleX;
    float m_scaleY;
    float m_offsetX;
    float m_offsetY;

    void calculateScaleAndOffset();
};