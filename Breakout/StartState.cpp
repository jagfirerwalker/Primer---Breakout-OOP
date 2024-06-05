#include "StartState.h"
#include "Constants.h"
#include <iostream>

StartState::StartState(GLFWwindow* window, std::map<std::string, GLuint>& fonts, std::map<std::string, GLuint>& sounds)
	: m_window(window), m_fonts(fonts), m_sounds(sounds), m_Highlighted(1)
{}

void StartState::update(float dt)
{
	// Toggle highlighted option if we press an arrow key up or down
	if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		m_Highlighted = (m_Highlighted == 1) ? 2 : 1;
		// Play the 'paddle hit' sound
		// TODO: Implement sound playback using OpenAL or similar library
	}

	// We no longer have this globally defined, so we need to pass it in
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_window, true);
	}
}

void StartState::render()
{
	//Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, 0, -1, 1);

	// Set up the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Render the title
	// TODO: Render the title using a font rendering library like FreeType

	// Render the instructions
	// TODO: Render the 'START' and 'HIGH SCORES' options using a font rendering library like FreeType

	// Highlight the selected option
	if (m_Highlighted == 1)
	{
		// TODO: Set the color of the 'START' option to blue
	}
	else if (m_Highlighted == 2)
	{
		// TODO: Set the color of the 'HIGH SCORES' option to blue
	}

	glfwSwapBuffers(m_window);
}