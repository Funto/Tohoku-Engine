// PhotonsMap.h

#ifndef PHOTONS_MAP_H
#define PHOTONS_MAP_H

#include "../../Common.h"
#include "../../glutil/glxw.h"
#include "../../scene/LightData.h"
#include "../../scene/GPUProgramManager.h"

class GBuffer;
class BounceMap;

class PhotonsMap : public LightData
{
private:
	uint size;

	GLuint id_vao;	// VAO/VBO for the rectangle:
	GLuint id_vbo;

	GPUProgramRef program;	// GPU program

	GLuint id_fbo;	// FBO

	GLuint id_output0;		// RGBA16F: see PHOTONS_MAP.frag
	GLuint id_output1;		// RGBA16F: see PHOTONS_MAP.frag

public:
	PhotonsMap(uint size);
	virtual ~PhotonsMap();

	virtual Type getType() const {return PHOTONS_MAP;}

	void compute(const BounceMap* bounce_map, const GBuffer* gbuffer, uint nb_iterations=1);

	// Getters:
	inline uint getSize() const  {return size;}

	inline GLuint getTexOutput0()  const {return id_output0;}
	inline GLuint getTexOutput1()  const {return id_output1;}

	inline GLuint getTexOutput(uint i) const {return (i == 0 ? id_output0 : id_output1);}

private:
	void createFBO();
	void createProgram();
	void createVBOAndVAO();
};

#endif // PHOTONS_MAP_H
