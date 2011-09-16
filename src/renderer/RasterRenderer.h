// RasterRenderer.h

#ifndef RASTER_RENDERER_H
#define RASTER_RENDERER_H

#include "Renderer.h"
#include "../Common.h"
#include "../CommonIndices.h"
#include "../Debug.h"
#include "../scene/Material.h"
#include <list>
#include <string>

class Light;
class Camera;
class ArrayElementContainer;
class Scene;
class GeneralProfile;
class TexunitManager;
struct TextureBinding;

class RasterRenderer : public Renderer
{
private:
	bool use_shadow_mapping;	// NB: current design only allows to set the usage of shadow mapping
								// at creation/deletion of the renderer.

	bool use_visibility_maps;	// This has a meaning only when RasterRenderer is used for rendering to a GBuffer.

#ifdef USE_DEBUG_TEXTURE
	GLuint id_debug;
#endif

	std::string brdf_function;

	uint vao_index;
	uint vao_index_shadow;

	uint profile_index;

public:
	RasterRenderer(uint width, uint height,
				   const vec3& back_color,
				   bool use_shadow_mapping,
				   bool use_visibility_maps,
				   const std::string& brdf_function,
				   uint vao_index        = VAO_INDEX_RASTER,
				   uint vao_index_shadow = VAO_INDEX_RASTER_SHADOW,
				   uint profile_index    = GENERAL_PROFILE);
	virtual ~RasterRenderer();

	// Called when we switch to this renderer
	virtual void setup();

	// Called when we switch to another renderer or close the program
	virtual void cleanup();

	// Called when we change the scene
	virtual void loadSceneArray(Scene* scene);
	void loadSceneArrayExt(Scene* scene, const Preprocessor::SymbolList& additional_preproc_syms);
	virtual void unloadSceneArray(Scene* scene);

	// Render one frame
	// If light_viewpoint != NULL, render from the viewpoint of the given light.
	virtual void renderArray(Scene* scene);
	void renderArrayExt(Scene* scene,
						const vec3& back_color,
						const Light* light_viewpoint=NULL,
						TextureBinding* added_tex_bindings=NULL,
						uint nb_added_tex_bindings=0);

	// Debug drawing
	virtual void debugDraw2D(Scene* scene);
	virtual void debugDraw3D(Scene* scene);

	// Get the renderer's name
	virtual const char* getName() const {return "RasterRenderer";}

private:
	// This function:
	// - binds the shadow map textures
	// - sets the uniforms for indicating the texture units
	// - uses and updates the texunit manager
	// It is called from the rendering function.
	void bindShadowMaps(Light** lights,
						uint nb_lights,
						glutil::GPUProgram* program,
						TexunitManager* texunit_manager) const;
};

#endif // RASTER_RENDERER_H
