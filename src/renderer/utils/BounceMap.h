// BounceMap.h

#ifndef BOUNCE_MAP_H
#define BOUNCE_MAP_H

#include "../../Common.h"
#include "../../glutil/glxw.h"
#include "../../scene/LightData.h"
#include "../../scene/GPUProgramManager.h"

class Camera;
class Light;
class Scene;
class RasterRenderer;
class ArrayElementContainer;
class GBuffer;

class BounceMap : public LightData
{
private:
	uint size;

	GLuint id_vao;	// VAO/VBO for the rectangle:
	GLuint id_vbo;

	GPUProgramRef program;	// GPU program

	GLuint id_fbo;	// FBO

	GLuint id_output0;		// RGBA16F: see bounce_map.frag
	GLuint id_output1;		// RGBA16F: see bounce_map.frag

	// Occlusion query:
//	GLuint id_query;
//	bool should_get_query_result;
//	uint nb_surviving_photons;

public:
	BounceMap(uint size);
	virtual ~BounceMap();

	virtual Type getType() const {return BOUNCE_MAP;}

	// Rendering:
	void renderFromGBuffer(const GBuffer* gbuffer, int num_iteration=0);

	// Getters:
	inline uint getSize() const  {return size;}

	inline GLuint getTexOutput0()  const {return id_output0;}
	inline GLuint getTexOutput1()  const {return id_output1;}

	inline GLuint getTexOutput(uint i) const {return (i == 0 ? id_output0 : id_output1);}

//	uint getNbSurvivingPhotons();

private:
	void createFBO();
	void createProgram();
	void createVBOAndVAO();
};

#endif // BOUNCE_MAP_H
