// main.cpp

#include "Application.h"
#include "log/Log.h"

int main(int argc, char* argv[])
{
	Log::open(argc, argv);

	Application* app = new Application(argc, argv);
	int ret = app->run();
	delete app;

	Log::close();
	return ret;
}
