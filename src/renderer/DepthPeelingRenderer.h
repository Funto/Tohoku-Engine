// DepthPeelingRenderer.h
// This class is DEPRECATED.
// TODO: when used normally (not independently), the first normal texture is unused.

#ifndef DEPTH_PEELING_RENDERER_H
#define DEPTH_PEELING_RENDERER_H

#include "Renderer.h"
#include "../Common.h"
#include "../scene/Material.h"

class Camera;
class Object;
class ArrayElementContainer;
class RasterRenderer;

class DepthPeelingRenderer : public Renderer
{
private:
	// Layers computed by the depth peeling.
	// When using the depth peeling renderer as a single independent renderer,
	// the first layer is just cleared and serves as a base layer for the other ones
	// (this is for debugging only).
	GLuint* id_depth_layers;	// DEPTH_COMPONENT
	GLuint* id_normal_layers;	// RGBA8
	uint nb_layers;

	GLuint id_fbo;

public:
	DepthPeelingRenderer(uint width, uint height, const vec3& back_color, uint nb_layers);
	virtual ~DepthPeelingRenderer();

	// Called when we switch to this renderer
	virtual void setup();

	// Called when we switch to another renderer or close the program
	virtual void cleanup();

	// Called when we change the current scene
	virtual void loadSceneArray(Scene* scene);
	void loadSceneArrayExt(Scene* scene, bool sort_objects);
	virtual void unloadSceneArray(Scene* scene);

	// Render one frame
	virtual void renderArrayExt(Scene* scene, GLuint id_front_depth=0);
	virtual void renderArray(Scene* scene);

	// Debug drawing
	virtual void debugDraw2D(Scene* scene);
	void debugDraw2DExt(Scene* scene, GLuint id_front_depth=0, GLuint id_front_normal=0);

	// Get the renderer's name
	virtual const char* getName() const {return "DepthPeelingRenderer";}

private:
	void singlePassArray(Object** objects,
						 uint nb_objects,
						 const mat4& view_matrix,
						 const mat4& proj_matrix);
};

#endif // DEPTH_PEELING_RENDERER_H
