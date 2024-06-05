#include "Push.h"
#include "Constants.h"
#include "StateMachine.h"
#include "StartState.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <string>

// Global variables
std::map<std::string, GLuint> gFonts;
std::map<std::string, GLuint> gTextures;
std::map<std::string, GLuint> gSounds;
std::unique_ptr<StateMachine> gStateMachine;
std:map<int, bool> gKeysPressed;

void loadResources() 
{
	// Load fonts
	gFonts["small"] = loadFont("assets/fonts/font.ttf", 8);
	gFonts["medium"] = loadFont("assets/fonts/font.ttf", 16);
	gFonts["large"] = loadFont("assets/fonts/font.ttf", 32);

	// Load textures
	gTextures["background"] = loadTexture("assets/graphics/background.png");
	gTextures["main"] = loadTexture("assets/graphics/breakout.png");
	gTextures["arrows"] = loadTexture("assets/graphics/arrows.png");
	gTextures["hearts"] = loadTexture("assets/graphics/hearts.png");
	gTextures["particle"] = loadTexture("assets/graphics/particle.png");

	// Load sounds buffers
	gSounds["paddle-hit"] = loadSound("assets/sounds/paddle_hit.wav");
	gSounds["score"] = loadSound("assets/sounds/score.wav");
	gSounds["wall-hit"] = loadSound("assets/sounds/wall_hit.wav");
	gSounds["confirm"] = loadSound("assets/sounds/confirm.wav");
	gSounds["select"] = loadSound("assets/sounds/select.wav");
	gSounds["no-select"] = loadSound("assets/sounds/no-select.wav");
	gSounds["brick-hit-1"] = loadSound("assets/sounds/brick-hit-1.wav");
	gSounds["brick-hit-2"] = loadSound("assets/sounds/brick-hit-2.wav");
	gSounds["hurt"] = loadSound("assets/sounds/hurt.wav");
	gSounds["victory"] = loadSound("assets/sounds/victory.wav");
	gSounds["recover"] = loadSound("assets/sounds/recover.wav");
	gSounds["high-score"] = loadSound("assets/sounds/high_score.wav");
	gSounds["pause"] = loadSound("assets/sounds/pause.wav");

	gSounds["music"] = loadSound("assets/sounds/music.wav");

}

int main()
{
	Push push;
	push.setupScreen(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, true, false, false);

	while (!glfwWindowShouldClose(push.getWindow()))
	{
		push.start();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		push.finish();
		glfwPollEvents();
	}

	return 0;
}
