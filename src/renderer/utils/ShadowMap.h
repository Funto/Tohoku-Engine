// ShadowMap.h
// A shadow map is a square depth texture.

#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "../../Common.h"
#include "../../scene/GPUProgramManager.h"
#include "../../scene/LightData.h"
#include "../../glutil/glxw.h"

class Light;
class Scene;
class ArrayElementContainer;

class ShadowMap : public LightData
{
private:
	uint size;

	GLuint id_fbo;

	GLuint id_rb_color;		// RGBA8: Color renderbuffer (necessary for completeness of the FBO)
	GLuint id_depth;		// DEPTH_COMPONENT

	GPUProgramRef program;

public:
	ShadowMap(uint size);
	virtual ~ShadowMap();

	// Implementation of the LightData interface:
	virtual Type getType() const {return SHADOW_MAP;}

	// We need to call these functions when loading / unloading a scene.
	// They create/delete VAOs necessary for rendering the shadow map from
	// the light and assign/remove shadow maps to all lights.
	static void loadScene(Scene* scene, uint index_vao);
	static void unloadScene(Scene* scene, uint index_vao);

	// Array versions:
	static void loadSceneArray(ArrayElementContainer* elements, uint index_vao);
	static void unloadSceneArray(ArrayElementContainer* elements, uint index_vao);

	// Rendering of all shadow maps:
	static void renderShadowMaps(Scene* scene, uint vao_index);

	// Array version:
	static void renderShadowMapsArray(ArrayElementContainer* elements, uint vao_index);

	// Render a scene from the point of view of a light
	void renderFromLight(const Light* light, Scene* scene, uint index_vao);
	void renderFromLightArray(const Light* light, ArrayElementContainer* elements, uint index_vao);

	inline uint getSize() const  {return size;}
	inline GLuint getTexDepth()     const {return id_depth;}

private:
	void setupGPUProgram();
};

#endif // SHADOW_MAP_H
