#ifndef __GUI_H__
#define __GUI_H__

class GUI
{
public:
	GUI();
	~GUI();

	void Start();
	void PreUpdate();
	void Update();
	void PostUpdate();
	void CleanUp();
};

#endif /* __GUI_H__ */