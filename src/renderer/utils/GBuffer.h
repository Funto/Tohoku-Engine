// GBuffer.h

#ifndef GBUFFER_H
#define GBUFFER_H

#include "../../Common.h"
#include "../../glutil/glxw.h"
#include "../../scene/LightData.h"

class Camera;
class Light;
class Scene;
class RasterRenderer;
struct TextureBinding;

class GBuffer : public LightData
{
private:
	uint width, height;
	bool use_visibility_maps;

	GLuint id_fbo;

	GLuint id_position;		// RGBA16F: x   y   z   free
	GLuint id_normal;		// RGBA16F: nx  ny  nz  free
	GLuint id_diffuse;		// RGBA16F: dr  dg  db  alpha
	GLuint id_specular;		// RGBA16F: sr  sg  sb  exponent
	GLuint id_depth;		// DEPTH_COMPONENT

	GLuint id_visibility;	// RGBA8: visibility for the light i is indicated in the ith component.
							// Only used when shadow mapping is enabled.

public:
	GBuffer(uint width, uint height, bool use_visibility_maps);
	virtual ~GBuffer();

	virtual Type getType() const {return GBUFFER;}

	void render(Scene* scene,
				RasterRenderer* raster_renderer,
				TextureBinding* added_tex_bindings=NULL,
				uint nb_added_tex_bindings=0);
	void renderFromLight(const Light* light,
						 Scene* scene,
						 RasterRenderer* raster_renderer,
						 TextureBinding* added_tex_bindings=NULL,
						 uint nb_added_tex_bindings=0);

	inline uint getWidth() const  {return width;}
	inline uint getHeight() const {return height;}

	inline GLuint getFBO()           const {return id_fbo;}
	inline GLuint getTexPositions()  const {return id_position;}
	inline GLuint getTexNormals()    const {return id_normal;}
	inline GLuint getTexDiffuse()    const {return id_diffuse;}
	inline GLuint getTexSpecular()   const {return id_specular;}
	inline GLuint getTexDepth()      const {return id_depth;}
	inline GLuint getTexVisibility() const {return id_visibility;}
};

#endif // GBUFFER_H
