#include "StateMachine.h"
#include <cassert>

StateMachine::StateMachine()
{
	states["empty"] = []() { return new State(); };
	currentState = states["empty"](); // Create a new State object
}

StateMachine::~StateMachine()
{
	delete currentState;
}

void StateMachine::addState(const std::string& name, StateCallback callback)
{
	states[name] = callback;
}

template <typename T>
void StateMachine::addState(const std::string& name)
{
	states[name] = []() { return new T(); };
}


void StateMachine::changeState(const std::string& name)
{
	assert(states.find(name) != states.end()); // Check if the state exists
	currentState->exit(); // Exit the current state
	delete currentState; // Delete the current state
	currentState = states[name](); // Create a new state
	currentState->enter(); // Enter the new state
}

void StateMachine::update(float dt)
{
	currentState->update(dt);
}

void StateMachine::render()
{
	currentState->render();
}