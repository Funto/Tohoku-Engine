// GLFWWindow.h

#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include "../Common.h"
#include "../glutil/glxw.h"
#include <GLFW/glfw3.h>
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
	GLFWwindow* window;

	typedef std::list<EventReceiver*> EventReceiverList;
	EventReceiverList event_receivers;

	static void errorCallback(int error, const char* description);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	GLFWWindow();

public:
	static GLFWWindow* getInstance();

	bool open(const std::string& title, uint width, uint height);
	void close();
	void swapBuffers();

	void enableVSync(bool enable=true);

	GLFWwindow* getWindow() const {return window;}

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
