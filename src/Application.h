// Application.h

#ifndef APPLICATION_H
#define APPLICATION_H

#include "gui/GLFWWindow.h"
#include "Common.h"
#include <vector>

class Scene;
class Renderer;
class CameraAnimator;
class SceneAnimator;

class Application : public EventReceiver
{
private:
	typedef std::vector<Scene*> SceneList;
	typedef std::vector<Renderer*> RendererList;
	typedef std::vector<CameraAnimator*> CameraAnimatorList;
	typedef std::vector<SceneAnimator*> SceneAnimatorList;

	SceneList scenes;
	uint num_current_scene;

	RendererList renderers;
	uint num_current_renderer;

	CameraAnimatorList camera_animators;
	uint num_current_camera_animator;

	SceneAnimatorList scene_animators;
	uint num_current_scene_animator;

	int fps;

	bool debug_draw_2D;
	bool debug_draw_3D;

public:
	Application(int argc, char* argv[]);
	int run();

private:
	void render();
	void update(double elapsed);

	void countFPS();
	void updateWinTitle(double render_time);

	void createScenes();
	void createRenderers();
	void createCameraAnimators();
	void createSceneAnimators();

	void cleanup();

	void printHelp();

	virtual void onKeyEvent(int key, int action);
};

#endif // APPLICATION_H
