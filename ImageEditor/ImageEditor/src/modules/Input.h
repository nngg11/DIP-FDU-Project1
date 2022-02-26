#ifndef __INPUT_H__
#define __INPUT_H__

#define NUM_MOUSE_BUTTONS 5

enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP,
};

class Input
{
public:
	Input();
	~Input();

	void Start();
	void PreUpdate();
	void Update();
	void PostUpdate();
	void CleanUp();

public:
	KEY_STATE GetMouseButton(const int& id);
	void GetMousePosition(int* x, int* y);

private:
	void UpdateMouseState();

public:
	KEY_STATE mouse_buttons[NUM_MOUSE_BUTTONS];
};

#endif /* __INPUT_H__ */