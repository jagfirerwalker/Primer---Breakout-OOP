#pragma once

class BaseState
{
public:
	virtual ~BaseState() {}

	virtual void enter() {}
	virtual void exit() {}
	virtual void update(float dt) {}
	virtual void render() {}
};
