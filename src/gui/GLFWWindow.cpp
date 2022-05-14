// GLFWWindow.cpp

#include "GLFWWindow.h"
#include "../log/Log.h"
#include "../glutil/glutil.h"

GLFWWindow::GLFWWindow()
: title(""), width(0), height(0), opened(false), event_receivers()
{
}

GLFWWindow* GLFWWindow::getInstance()
{
	static GLFWWindow win;
	return &win;
}

bool GLFWWindow::open(const std::string& title, uint width, uint height)
{
	if(opened)
		return true;

	this->title = title;
	this->width = width;
	this->height = height;

	if(glfwInit() == GL_FALSE)
	{
		logError("cannot initialize GLFW");
		return false;
	}

	glfwSetErrorCallback(&errorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if(!window)
	{
		logError("cannot open the GLFW window");
		return false;
	}

	glfwMakeContextCurrent(window);

	if(!glxwInit())
		return false;

	glfwSetMouseButtonCallback(window, &mouseButtonCallback);
	glfwSetCursorPosCallback(window, &cursorPosCallback);
	glfwSetScrollCallback(window, &scrollCallback);
	glfwSetKeyCallback(window, &keyCallback);

	this->opened = true;

	return true;
}

void GLFWWindow::close()
{
	if(!opened)
		return;

	glfwDestroyWindow(window);
	glfwTerminate();
	opened = false;
}

void GLFWWindow::swapBuffers()
{
	if(!opened)
		return;

	glfwSwapBuffers(window);
	glfwPollEvents();

	GL_CHECK();

	if(glfwGetKey(window, GLFW_KEY_ESCAPE) || glfwWindowShouldClose(window))
		close();
}

void GLFWWindow::enableVSync(bool enable)
{
	if(enable)
		glfwSwapInterval(1);	// We want at least one vertical retrace between each buffer swap
	else
		glfwSwapInterval(0);
}

// Callback wrappers:
// ---------------------------------------------------------------------
void GLFWWindow::errorCallback(int error, const char* description)
{
	logError("GLFW error: ", description);
}

void GLFWWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	EventReceiverList& event_receivers = GLFWWindow::getInstance()->event_receivers;

	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		EventReceiver* event_receiver = (*it);
		event_receiver->onMouseButtonEvent(button, action);
	}
}

void GLFWWindow::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	EventReceiverList& event_receivers = GLFWWindow::getInstance()->event_receivers;

	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		EventReceiver* event_receiver = (*it);
		event_receiver->onMousePosEvent(xpos, ypos);
	}
}

void GLFWWindow::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	EventReceiverList& event_receivers = GLFWWindow::getInstance()->event_receivers;

	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		EventReceiver* event_receiver = (*it);
		event_receiver->onMouseWheelEvent(yoffset);
	}
}

void GLFWWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	EventReceiverList& event_receivers = GLFWWindow::getInstance()->event_receivers;

	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		EventReceiver* event_receiver = (*it);
		event_receiver->onKeyEvent(key, action);
	}
}

// End callback wrappers
// ---------------------------------------------------------------------

void GLFWWindow::addEventReceiver(EventReceiver* receiver)
{
	this->event_receivers.push_back(receiver);
}

void GLFWWindow::removeEventReceiver(EventReceiver* receiver)
{
	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		if((*it) == receiver)
		{
			event_receivers.erase(it);
			break;
		}
	}
}

void GLFWWindow::cleanEventReceivers()
{
	this->event_receivers.clear();
}
