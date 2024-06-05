#pragma once
#include "BaseState.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <string>

class StartState : public BaseState
{
public:
	StartState(GLFWwindow* window, std::map<std::string, GLuint>& fonts, std::map<std::string, GLuint>& sounds);

	virtual void update(float dt) override;
	virtual void render() override;

private:
	GLFWwindow* m_window;
	std::map<std::string, GLuint>& m_fonts;
	std::map<std::string, GLuint>& m_sounds;

	int m_Highlighted;
};
