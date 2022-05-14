// main.cpp

#include "Application.h"
#include "log/Log.h"

#ifdef _MSC_VER
	#ifdef _M_X64
		#error "TODO: link x64 libraries"
	#else
		#ifdef NDEBUG
			#pragma comment(lib, R"(opengl32.lib)")
			#pragma comment(lib, R"(..\lib\pthreadVCE2.lib)")
			#pragma comment(lib, R"(..\lib\OpenCL.lib)")
			#pragma comment(lib, R"(..\..\glfw-3.3.7.bin.WIN32\lib-vc2019\glfw3dll.lib)")
		#else
			#pragma comment(lib, R"(opengl32.lib)")
			#pragma comment(lib, R"(..\lib\pthreadVCE2.lib)")
			#pragma comment(lib, R"(..\lib\OpenCL.lib)")
			#pragma comment(lib, R"(..\..\glfw-3.3.7.bin.WIN32\lib-vc2019\glfw3dll.lib)")
		#endif
	#endif
#endif

int main(int argc, char* argv[])
{
	Log::open(argc, argv);

	Application* app = new Application(argc, argv);
	int ret = app->run();
	delete app;

	Log::close();
	return ret;
}
