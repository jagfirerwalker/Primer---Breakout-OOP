#pragma once
#include <map>
#include <string>
#include <functional> // include the functional library
#include <cassert> // include the cassert library, which is used for debugging purposes

// Create a State class with virtual functions
// Virtual functions are function that are declared in the base class and are redefined in the derived class
class State
{
public:
	// Create a virtual destructor for the State class
	virtual ~State() {}
	virtual void enter(void* params) {}
	virtual void exit() {}
	virtual void update(float dt) {}
	virtual void render() {}
};

// Create a StateMachine class
class StateMachine
{
public:
	using StateCallback = std::function<State*()>; // Create a StateCallback type that is a function that returns a State pointer

	StateMachine(); // Create a constructor for the StateMachine class
	~StateMachine(); // Create a destructor for the StateMachine class


	void addState(const std::string& name, StateCallback callback); // Create a function that adds a state to the state machine

	template <typename T> // Create a template function that adds a state to the state machine
	void addState(const std::string& name); // Create a function that adds a state to the state machine

	void changeState(const std::string& name);	
	void update(float dt);
	void render();

private:
	std::map<std::string, StateCallback> states;
	State* currentState = nullptr;
};
