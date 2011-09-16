// RaytraceRenderer.h

#ifndef RAYTRACE_RENDERER_H
#define RAYTRACE_RENDERER_H

#include "Renderer.h"
#include "../Common.h"

namespace glutil
{
	class Quad;
}

class Camera;
class ArrayElementContainer;
class Sphere;
class Material;
class Light;

class RaytraceRenderer : public Renderer
{
private:
	struct Ray
	{
		vec3 start;
		vec3 direction;

		Ray() {}
		Ray(const vec3& start, const vec3& direction)
		: start(start), direction(direction)
		{
		}
	};

	struct Triangle
	{
		vec3 v0, v1, v2;
		vec3 normal;
		Material* material;
	};

	struct TriangleContainer
	{
		Triangle* triangles;	// Owned
		uint nb_triangles;

		TriangleContainer() : triangles(NULL), nb_triangles(0)
		{
		}

		~TriangleContainer()
		{
			delete [] triangles;
		}
	};

private:
	glutil::Quad* fs_quad;
	TriangleContainer tri_container;	// Cached triangles
	Light** lights;	// Cached information about lights
	uint nb_lights;
	uint max_depth;	// Maximum recursion depth
	float min_reflection;	// Minimum reflection coefficient

	// Multithreading :
	bool use_multithread;
	int index_next_pixel[2];

public:
	RaytraceRenderer(uint width, uint height, const vec3& back_color);
	virtual ~RaytraceRenderer();

	// Called when we switch to this renderer
	virtual void setup();

	// Called when we switch to another renderer or close the program
	virtual void cleanup();

	// Render one frame
	virtual void renderArray(Scene* scene);

	// Get the renderer's name
	virtual const char* getName() const {return "RaytraceRenderer";}

	// Implementation of KeyEventReceiver :
	virtual void onKeyEvent(int key, int action);

private:
	// Copy from an ArrayElementContainer to an optimized TriangleContainer while applying tranformations
	void fillTriangleContainerArray(const ArrayElementContainer* elements);

	// Rendering in case the ElementContainer is an ArrayElementContainer
	// - single-threaded version :
	void renderArraySinglethread(Pixel* pixels, const Camera* camera, const ArrayElementContainer* elements);

	// - multi-threaded version
	void renderArrayMultithread(Pixel* pixels, const Camera* camera, const ArrayElementContainer* elements);
public:
	void renderArrayThread(	Pixel* pixels, const Camera* camera,
							const ArrayElementContainer* elements,
							int thread_id, float dx, float dy);
private:

	// Launching one ray and get the distance to the intersection
	Triangle* launchRay(const Ray& r, float* pt) const;

	// Recursive launching of rays resulting in a color value
	vec3 launchColorRay(const Ray& r, uint depth=0) const;

	// Ray / triangle intersection code
	inline bool rayHitsTriangle(const Ray& r, const vec3& v0, const vec3& v1, const vec3& v2, float* t) const;

	// Ray / sphere intersection code
	inline bool rayHitsSphere(const Ray& r, const Sphere* sphere, float* t) const;
};

#endif // RAYTRACE_RENDERER_H
