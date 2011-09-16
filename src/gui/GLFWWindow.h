// GLFWWindow.h

#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include "../Common.h"
#include "../glutil/glxw.h"
#include <GL/glfw.h>
#include <string>
#include <list>

class EventReceiver;

class GLFWWindow
{
private:
	std::string title;
	uint width;
	uint height;
	bool opened;

	typedef std::list<EventReceiver*> EventReceiverList;
	EventReceiverList event_receivers;

	static void GLFWCALL mouseButtonCallback(int button, int action);
	static void GLFWCALL mousePosCallback(int x, int y);
	static void GLFWCALL mouseWheelCallback(int pos);
	static void GLFWCALL keyCallback(int key, int action);

private:
	GLFWWindow();

public:
	static GLFWWindow* getInstance();

	bool open(const std::string& title, uint width, uint height);
	void close();
	void swapBuffers();

	void enableVSync(bool enable=true);

	const std::string& getTitle() const {return title;}
	uint getWidth() const {return width;}
	uint getHeight() const {return height;}

	bool isOpened() const {return opened;}

	void addEventReceiver(EventReceiver* receiver);
	void removeEventReceiver(EventReceiver* receiver);
	void cleanEventReceivers();
};

class EventReceiver
{
public:
	virtual void onMouseButtonEvent(int button, int action) {}
	virtual void onMousePosEvent(int x, int y) {}
	virtual void onMouseWheelEvent(int pos) {}
	virtual void onKeyEvent(int key, int action) {}
};

#endif // GLFW_WINDOW_H
