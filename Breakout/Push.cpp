#include "Push.h"
#include <iostream>

Push::Push()
    : m_virtualWidth(0), m_virtualHeight(0), m_windowWidth(0), m_windowHeight(0),
    m_fullscreen(false), m_resizable(false), m_window(nullptr),
    m_scaleX(1.0f), m_scaleY(1.0f), m_offsetX(0.0f), m_offsetY(0.0f) {
}

Push::~Push() {
    // Cleanup
    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}
void Push::setupScreen(int virtualWidth, int virtualHeight, int windowWidth, int windowHeight, bool vsync, bool fullscreen, bool resizeable)
{
    m_virtualWidth = virtualWidth;
    m_virtualHeight = virtualHeight;
    m_windowWidth = virtualWidth;
    m_windowHeight = virtualHeight;
    m_fullscreen = fullscreen;
    m_resizable = resizeable;

    // Initialize GLFW
    if (!glfwInit()) {
		// Error handling
        std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

    // Set up GLFW window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, m_resizable ? GL_TRUE : GL_FALSE);

    m_window = glfwCreateWindow(windowWidth, windowHeight, "Breakout", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
    
    if (!m_window)
    {
        // Handle window creation failure
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(vsync ? 1 : 0);

    // Initalize GLEW
    if (glewInit() != GLEW_OK)
    { 
        // Handle GLEW initialization failure
		std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(m_window);
        glfwTerminate();
		return;
    }

    calculateScaleAndOffset();

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
        {
            Push* push = static_cast<Push*>(glfwGetWindowUserPointer(window));
            if (push)
            {
                push->resize(width, height);
            }
        });
}

void Push::calculateScaleAndOffset()
{
    m_scaleX = static_cast<float>(m_windowWidth) / m_virtualWidth;
    m_scaleY = static_cast<float>(m_windowHeight) / m_virtualHeight;

    if (m_scaleX < m_scaleY)
    {
        m_scaleY = m_scaleX;
    }
    else
    {
        m_scaleX = m_scaleY;
    }

    m_offsetX = (m_windowWidth - (m_virtualWidth * m_scaleX)) * 0.5f;
    m_offsetY = (m_windowHeight - (m_virtualHeight * m_scaleY)) * 0.5f;
}

void Push::start()
{
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_virtualWidth, m_virtualHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(m_offsetX / m_scaleX, m_offsetY / m_scaleY, 0.0f);
    glScalef(m_scaleX, m_scaleY, 1.0f);
}

void Push::finish()
{
    glfwSwapBuffers(m_window);
}

void Push::resize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;

    calculateScaleAndOffset();
    glViewport(0, 0, m_windowWidth, m_windowHeight);
}

int Push::getWidth() const
{
	return m_virtualWidth;
}

int Push::getHeight() const
{
	return m_virtualHeight;
}

GLFWwindow* Push::getWindow() const
{
	return m_window;
}