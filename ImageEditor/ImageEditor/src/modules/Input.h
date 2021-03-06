#ifndef __INPUT_H__
#define __INPUT_H__

#include "Module.h"

#define NUM_MOUSE_BUTTONS 5

enum class KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP,
};

class Input : public Module
{
public:
	Input();
	~Input();

	void Start() override;
	void PreUpdate() override;
	void Update() override;
	void PostUpdate() override;
	void CleanUp() override;

public:
	KEY_STATE GetMouseButton(const int& id) const;
	void GetMousePosition(int& x, int& y) const;
	void GetMouseMotion(int& x, int& y) const;

private:
	void UpdateMouseState();

public:
	KEY_STATE mouse_buttons[NUM_MOUSE_BUTTONS];

private:
	int mouse_position_x;
	int mouse_position_y;
	int mouse_motion_x;
	int mouse_motion_y;
};

#endif /* __INPUT_H__ */