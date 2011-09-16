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

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR,  3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR,  3);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	if(!glfwOpenWindow(width, height, 8, 8, 8, 8, 24, 8, GLFW_WINDOW))
	{
		logError("cannot open the GLFW window");
		return false;
	}

	if(!glxwInit())
		return false;

	glfwSetWindowTitle(title.c_str());

	glfwSetMouseButtonCallback(&mouseButtonCallback);
	glfwSetMousePosCallback(&mousePosCallback);
	glfwSetMouseWheelCallback(&mouseWheelCallback);
	glfwSetKeyCallback(&keyCallback);

	this->opened = true;

	return true;
}

void GLFWWindow::close()
{
	if(!opened)
		return;

	glfwCloseWindow();
	glfwTerminate();
	opened = false;
}

void GLFWWindow::swapBuffers()
{
	if(!opened)
		return;

	glfwSwapBuffers();

	GL_CHECK();

	if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
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
void GLFWCALL GLFWWindow::mouseButtonCallback(int button, int action)
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

void GLFWCALL GLFWWindow::mousePosCallback(int x, int y)
{
	EventReceiverList& event_receivers = GLFWWindow::getInstance()->event_receivers;

	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		EventReceiver* event_receiver = (*it);
		event_receiver->onMousePosEvent(x, y);
	}
}

void GLFWCALL GLFWWindow::mouseWheelCallback(int pos)
{
	EventReceiverList& event_receivers = GLFWWindow::getInstance()->event_receivers;

	for(EventReceiverList::iterator it = event_receivers.begin() ;
		it != event_receivers.end() ;
		it++)
	{
		EventReceiver* event_receiver = (*it);
		event_receiver->onMouseWheelEvent(pos);
	}
}

void GLFWCALL GLFWWindow::keyCallback(int key, int action)
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
