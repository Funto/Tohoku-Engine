// Application.cpp

#include "Application.h"
#include "Config.h"
#include "glutil/glutil.h"

#include "animators/FPSCameraAnimator.h"
#include "animators/KeyboardCameraAnimator.h"
#include "animators/NullSceneAnimator.h"
#include "animators/LightSceneAnimator.h"

#include "renderer/DebugRenderer.h"
#include "renderer/DeferredShadingRenderer.h"
#include "renderer/DepthPeelingRenderer.h"
#include "renderer/MultiLayerRenderer.h"
#include "renderer/MyRenderer.h"
#include "renderer/RasterRenderer.h"
#include "renderer/RaytraceRenderer.h"
#include "renderer/StencilRoutedRenderer.h"

#include "scene/Scene.h"
#include "scene/SceneLoader.h"
#include "scene/Camera.h"

#include "utils/Clock.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

Application::Application(int argc, char* argv[])
: scenes(), renderers(), fps(-1), debug_draw_2D(false), debug_draw_3D(true)
{
}

int Application::run()
{
	// Setup a window
	GLFWWindow* win = GLFWWindow::getInstance();

	if(!win->open(WIN_TITLE, WIN_WIDTH, WIN_HEIGHT))
		return EXIT_FAILURE;

	glutil::printGPUMemoryInfo("just after opening the window");

	win->addEventReceiver(this);

	win->enableVSync(ENABLE_VSYNC);

	// Initialize glutil:
	glutil::init();

	// Setup the scenes, the renderers and the camera animators
	createScenes();
	createRenderers();
	createCameraAnimators();
	createSceneAnimators();

	// Register the scene renderers as key event receivers
	for(uint i=0 ; i < renderers.size() ; i++)
		win->addEventReceiver(renderers[i]);

	// Main loop :
	double t = Clock::getSeconds();
	double t0 = Clock::getSeconds();

	while(win->isOpened())
	{
		// Update the scene
		t0 = Clock::getSeconds();
		update(t0 - t);
		t = t0;

		// Render
		render();

		// Update the number of FPS
		countFPS();

		// Update the window title
		updateWinTitle(Clock::getSeconds()-t);

		GL_CHECK();

		win->swapBuffers();
	}

	cleanup();

	// Shutdown glutil:
	glutil::shutdown();

	win->close();
	return EXIT_SUCCESS;
}

void Application::render()
{
	Renderer* current_renderer = renderers[num_current_renderer];
	Scene* current_scene = scenes[num_current_scene];

	current_renderer->render(current_scene);

	if(debug_draw_3D)
		current_renderer->debugDraw3D(current_scene);

	if(debug_draw_2D)
		current_renderer->debugDraw2D(current_scene);

	GL_CHECK();
}

void Application::update(double elapsed)
{
	CameraAnimator* camera_animator = camera_animators[num_current_camera_animator];
	camera_animator->update(elapsed);

	SceneAnimator* scene_animator = scene_animators[num_current_scene_animator];
	scene_animator->update(elapsed);
}

void Application::countFPS()
{
	static bool first_time = true;
	static uint nb_frames = 0;
	static double t0 = 0.0;

	if(first_time)
	{
		t0 = glfwGetTime();
		first_time = false;
	}

	nb_frames++;

	if(glfwGetTime() - t0 >= 1.0)
	{
		this->fps = int(nb_frames);
		t0 = glfwGetTime();
		nb_frames = 0;
	}
}

void Application::updateWinTitle(double render_time)
{
	stringstream ss;
	string scene_name = scenes[num_current_scene]->getName();
	string renderer_name = renderers[num_current_renderer]->getName();

	if(fps >= 0)
		ss	<< WIN_TITLE
			<< " | " << renderer_name
			<< " | FPS:" << fps
			<< " | " << scene_name
			<< " | " << render_time << "s/frame";
	else
		ss	<< WIN_TITLE
			<< " | " << renderer_name
			<< " | " << scene_name
			<< " | " << render_time << "s/frame";

	glfwSetWindowTitle(ss.str().c_str());
	ss.str("");
}

// Scenes
void Application::createScenes()
{
	Scene* scene;
	SceneLoader loader;

	cout << "---------- begin scenes creation ------------" << endl;

	// Cornell Suzanne
	scene = new Scene();
	loader.load(scene, "media/cornell_suzanne.dae");
	//loader.load(scene, "media/cornell_suzanne_2.dae");
	//loader.load(scene, "media/cornell_only.dae");
	scenes.push_back(scene);

	// Cube
	scene = new Scene();
	loader.load(scene, "media/cube.dae");
	scenes.push_back(scene);

	// Cube & walls
	scene = new Scene();
	loader.load(scene, "media/cube_walls.dae");
	scenes.push_back(scene);

	// Balls
	scene = new Scene();
	loader.load(scene, "media/balls.dae");
	scenes.push_back(scene);

	// Balls
	scene = new Scene();
	loader.load(scene, "media/caustics.dae");
	scenes.push_back(scene);

	// TODO: Sponza atrium
/*	// Sponza atrium
	scene = new Scene();
	loader.load(scene, "media/sponza_crytek_modif.obj");
	scenes.push_back(scene);
*/
	// Optionally override camera settings
#ifdef OVERRIDE_CAMERA_SETTINGS
	SceneList::iterator it_end = scenes.end();
	for(SceneList::iterator it = scenes.begin() ;
		it != it_end ;
		it++)
	{
		scene = (*it);
		scene->getCamera()->setProjection(CAMERA_FOVY,
										  CAMERA_ASPECT,
										  CAMERA_ZNEAR,
										  CAMERA_ZFAR);
	}
#endif

	num_current_scene = DEFAULT_SCENE;

	cout << "---------- end scenes creation ------------" << endl;
}

// Renderers
void Application::createRenderers()
{
	cout << "---------- begin renderers creation ------------" << endl;

	// 0: Debug renderer:
	renderers.push_back(new DebugRenderer(WIN_WIDTH, WIN_HEIGHT,
										  BACK_COLOR,
										  BRDF_FUNCTION));

	// 1: Forward shading renderer:
	renderers.push_back(new RasterRenderer(WIN_WIDTH, WIN_HEIGHT,
										   BACK_COLOR,
										   USE_SHADOW_MAPPING,
										   false,	// use_visibility_maps
										   BRDF_FUNCTION));

	// 2: Deferred shading renderer
	renderers.push_back(new DeferredShadingRenderer(WIN_WIDTH,
													WIN_HEIGHT,
													BACK_COLOR,
													USE_SHADOW_MAPPING,
													USE_VISIBILITY_MAPS,
													BRDF_FUNCTION));

	// 3: Depth peeling renderer (DEPRECATED)
	renderers.push_back(new DepthPeelingRenderer(WIN_WIDTH, WIN_HEIGHT, BACK_COLOR, NB_DEPTH_LAYERS));

	// 4: Stencil-routed A-Buffer renderer
	renderers.push_back(new StencilRoutedRenderer(WIN_WIDTH, WIN_HEIGHT));

	// 5: Multi layer renderer (deferred shading + depth peeling)
	renderers.push_back(new MultiLayerRenderer(WIN_WIDTH,
											   WIN_HEIGHT,
											   BACK_COLOR,
											   USE_SHADOW_MAPPING,
											   BRDF_FUNCTION,
											   NB_DEPTH_LAYERS));


	// 6: My own technique renderer:
	renderers.push_back(new MyRenderer(WIN_WIDTH,
									   WIN_HEIGHT,
									   BACK_COLOR,
									   USE_SHADOW_MAPPING,
									   USE_VISIBILITY_MAPS,
									   BRDF_FUNCTION,
									   KERNEL_FILENAME,
									   NB_DEPTH_LAYERS));

#ifdef CREATE_CPU_RAYTRACER
	// 7: Simple CPU raytracer:
	renderers.push_back(new RaytraceRenderer(WIN_WIDTH, WIN_HEIGHT, BACK_COLOR));
#endif

	num_current_renderer = DEFAULT_RENDERER;

	cout << "---------- end renderers creation ------------" << endl;

	// Setup the current renderer:
	Renderer* renderer = renderers[num_current_renderer];
	renderer->setup();

	// Load the current scene:
	renderer->loadScene(scenes[num_current_scene]);
}

// Camera animators
void Application::createCameraAnimators()
{
	// Get some pointers
	GLFWWindow* win = GLFWWindow::getInstance();
	Camera* camera = scenes[num_current_scene]->getCamera();

	// Create the camera animators
	camera_animators.push_back(new KeyboardCameraAnimator(camera));
	camera_animators.push_back(new FPSCameraAnimator(camera));

	// Set the default camera animator
	num_current_camera_animator = DEFAULT_CAMERA_ANIMATOR;

	// Add the animators as event receivers
	CameraAnimatorList::iterator it_end = camera_animators.end();
	for(CameraAnimatorList::iterator it = camera_animators.begin() ;
		it != it_end ;
		it++)
	{
		CameraAnimator* anim = (*it);
		win->addEventReceiver(anim);
	}
}

// Scene animators
void Application::createSceneAnimators()
{
	// Get some pointers
	GLFWWindow* win = GLFWWindow::getInstance();
	Scene* scene = scenes[num_current_scene];

	// Create the scene animators
	scene_animators.push_back(new NullSceneAnimator(scene));
	scene_animators.push_back(new LightSceneAnimator(scene));

	// Set the default scene animator
	num_current_scene_animator = DEFAULT_SCENE_ANIMATOR;

	// Add the animators as event receivers
	SceneAnimatorList::iterator it_end = scene_animators.end();
	for(SceneAnimatorList::iterator it = scene_animators.begin() ;
		it != it_end ;
		it++)
	{
		SceneAnimator* anim = (*it);
		anim->onSceneChanged();
		win->addEventReceiver(anim);
	}
}

// Cleanup
void Application::cleanup()
{
	// Cleanup renderers.
	// NB: some renderers can cleanup data related to scenes, so we must
	// cleanup renderers BEFORE cleaning up scenes.
	Renderer* renderer = renderers[num_current_renderer];

	renderer->unloadScene(scenes[num_current_scene]);
	renderer->cleanup();

	{
		RendererList::iterator it_end = renderers.end();
		for(RendererList::iterator it = renderers.begin() ;
			it != renderers.end() ;
			it++)
		{
			delete (*it);
		}
	}

	renderers.clear();

	// Cleanup scenes :
	{
		SceneList::iterator it_end = scenes.end();
		for(SceneList::iterator it = scenes.begin() ;
			it != it_end ;
			it++)
		{
			delete (*it);
		}
	}

	scenes.clear();

	// Cleanup the camera animators
	{
		CameraAnimatorList::iterator it_end = camera_animators.end();
		for(CameraAnimatorList::iterator it = camera_animators.begin() ;
			it != it_end ;
			it++)
		{
			delete (*it);
		}
	}

	// Cleanup the scene animators
	{
		SceneAnimatorList::iterator it_end = scene_animators.end();
		for(SceneAnimatorList::iterator it = scene_animators.begin() ;
			it != it_end ;
			it++)
		{
			delete (*it);
		}
	}
}

// Keyboard callback
static string nextValue(uint* pval, uint nb_values);
static string prevValue(uint* pval, uint nb_values);

void Application::onKeyEvent(int key, int action)
{
	uint nb_scenes = scenes.size();
	uint nb_renderers = renderers.size();
	uint nb_camera_animators = camera_animators.size();
	uint nb_scene_animators = scene_animators.size();

	if(action == GLFW_RELEASE)
	{
		Scene*          old_scene           = scenes           [num_current_scene];
		Renderer*       old_renderer        = renderers        [num_current_renderer];
		CameraAnimator* old_camera_animator = camera_animators [num_current_camera_animator];
		SceneAnimator*  old_scene_animator  = scene_animators  [num_current_scene_animator];
		string msg = "";

		// H: print help
		if(key == 'H')
			printHelp();

		else if(key == 'M')
			glutil::printGPUMemoryInfo();

		// O (Overlay): toggle 2D debug drawing on/off
		else if(key == 'O')
			debug_draw_2D = !debug_draw_2D;

		// D: toggle 3D debug drawing on/off
		else if(key == 'D')
			debug_draw_3D = !debug_draw_3D;

		// F1 / F2: change scene
		else if(key == GLFW_KEY_F1)
			msg = prevValue(&num_current_scene, nb_scenes);
		else if(key == GLFW_KEY_F2)
			msg = nextValue(&num_current_scene, nb_scenes);

		// F3 / F4: change renderer
		else if(key == GLFW_KEY_F3)
			msg = prevValue(&num_current_renderer, nb_renderers);
		else if(key == GLFW_KEY_F4)
			msg = nextValue(&num_current_renderer, nb_renderers);

		// F5 / F6: change camera animator
		else if(key == GLFW_KEY_F5)
			msg = prevValue(&num_current_camera_animator, nb_camera_animators);
		else if(key == GLFW_KEY_F6)
			msg = nextValue(&num_current_camera_animator, nb_camera_animators);

		// F7 / F8: change scene animator
		else if(key == GLFW_KEY_F7)
			msg = prevValue(&num_current_scene_animator, nb_scene_animators);
		else if(key == GLFW_KEY_F8)
			msg = nextValue(&num_current_scene_animator, nb_scene_animators);

		// ----------------------------------
		Scene*          new_scene           = scenes           [num_current_scene];
		Renderer*       new_renderer        = renderers        [num_current_renderer];
		CameraAnimator* new_camera_animator = camera_animators [num_current_camera_animator];
		SceneAnimator*  new_scene_animator  = scene_animators  [num_current_scene_animator];

		// Normally, scene and renderer can not be changed at the same time
		assert( ! (new_renderer != old_renderer && new_scene != old_scene) );

		// If we changed the scene:
		if(new_scene != old_scene)
		{
			assert(old_renderer == new_renderer);

			// Print information
			cout << msg << "scene : " << num_current_scene << "/" << scenes.size()
				 << " : \"" << new_scene->getName() << "\"" << endl;

			// Reset the initial state of the scene:
			old_scene_animator->reset();

			// Unload old scene and load new scene
			old_renderer->unloadScene(old_scene);
			new_renderer->loadScene(new_scene);

			// Update camera animators
			{
				CameraAnimatorList::iterator it_end = camera_animators.end();
				for(CameraAnimatorList::iterator it = camera_animators.begin() ;
					it != it_end ;
					it++)
				{
					(*it)->setCamera(new_scene->getCamera());
				}
			}

			// Update scene animators
			{
				SceneAnimatorList::iterator it_end = scene_animators.end();
				for(SceneAnimatorList::iterator it = scene_animators.begin() ;
					it != it_end ;
					it++)
				{
					(*it)->setScene(new_scene);
					(*it)->onSceneChanged();
				}
			}
		}

		// If we changed the renderer:
		else if(new_renderer != old_renderer)
		{
			assert(old_scene == new_scene);

			// Print information
			cout << msg << "renderer : " << num_current_renderer << "/" << scenes.size()
				 << " : \"" << new_renderer->getName() << "\"" << endl;

			// Unload scene
			old_renderer->unloadScene(old_scene);

			// Cleanup old renderer
			old_renderer->cleanup();

			// Setup new renderer
			new_renderer->setup();

			// Load scene
			new_renderer->loadScene(new_scene);
		}

		// If we changed the camera animator:
		else if(new_camera_animator != old_camera_animator)
		{
			// Print information
			cout << msg << "camera animator : " << num_current_camera_animator << "/" << camera_animators.size()
				 << " : \"" << new_camera_animator->getName() << "\"" << endl;

			// Notify the camera animators
			old_camera_animator->unload();
			new_camera_animator->load();
		}

		// If we changed the scene animator:
		else if(new_scene_animator != old_scene_animator)
		{
			// Print information
			cout << msg << "scene animator : " << num_current_scene_animator << "/" << scene_animators.size()
				 << " : \"" << new_scene_animator->getName() << "\"" << endl;

			// Notify the scene animators
			old_scene_animator->unload();
			new_scene_animator->load();
		}
	}
}

static string nextValue(uint* pval, uint nb_values)
{
	if(*pval == nb_values-1)
		*pval = 0;
	else
		(*pval)++;

	return "Changed to next ";
}

static string prevValue(uint* pval, uint nb_values)
{
	if(*pval == 0)
		*pval = nb_values-1;
	else
		(*pval)--;

	return "Changed to previous ";
}

void Application::printHelp()
{
	cout << "--------------" << endl;

	cout << "O (Overlay): toggle 2D debug drawing on/off [now: " << (debug_draw_2D ? "on" : "off") << "]" << endl;
	cout << "D: toggle 3D debug drawing on/off [now: " << (debug_draw_3D ? "on" : "off") << "]" << endl;
	cout << "M: print GPU memory information" << endl;
	cout << "--------------" << endl;

	cout << "F1: previous scene" << endl;
	cout << "F2: next scene" << endl;
	cout << "scenes: " << endl;
	for(uint i=0 ; i < scenes.size() ; i++)
	{
		if(i == num_current_scene)
			cout << "X ";
		else
			cout << "  ";
		cout << "[" << i << "] = \"" << scenes[i]->getName() << "\"" << endl;
	}

	cout << "--------------" << endl;

	cout << "F3: previous renderer" << endl;
	cout << "F4: next renderer" << endl;
	cout << "renderers: " << endl;
	for(uint i=0 ; i < renderers.size() ; i++)
	{
		if(i == num_current_renderer)
			cout << "X ";
		else
			cout << "  ";
		cout << "[" << i << "] = \"" << renderers[i]->getName() << "\"" << endl;
	}

	cout << "--------------" << endl;

	cout << "F5: previous camera animator" << endl;
	cout << "F6: next camera animator" << endl;
	cout << "camera animators: " << endl;
	for(uint i=0 ; i < camera_animators.size() ; i++)
	{
		if(i == num_current_camera_animator)
			cout << "X ";
		else
			cout << "  ";
		cout << "[" << i << "] = \"" << camera_animators[i]->getName() << "\"" << endl;
	}

	cout << "--------------" << endl;

	cout << "F7: previous scene animator" << endl;
	cout << "F8: next scene animator" << endl;
	cout << "scene animators: " << endl;
	for(uint i=0 ; i < scene_animators.size() ; i++)
	{
		if(i == num_current_scene_animator)
			cout << "X ";
		else
			cout << "  ";
		cout << "[" << i << "] = \"" << scene_animators[i]->getName() << "\"" << endl;
	}

	camera_animators[num_current_camera_animator]->printHelp();
	scene_animators[num_current_scene_animator]->printHelp();
}
