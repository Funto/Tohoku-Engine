// RaytraceRenderer.cpp

#include "RaytraceRenderer.h"
#include "../CommonIndices.h"
#include "../glutil/glutil.h"
#include "../log/Log.h"
#include "../scene/ArrayElementContainer.h"
#include "../scene/ElementContainer.h"
#include "../scene/Camera.h"
#include "../scene/Geometry.h"
#include "../scene/Light.h"
#include "../scene/Material.h"
#include "../scene/MeshObject.h"
#include "../scene/Scene.h"
#include "../scene/Sphere.h"
#include "../scene/profiles/RaytraceProfile.h"
#include <cstdlib>
#include <cmath>
#include <pthread.h>
#include <iostream>
using namespace std;

#define MAX_DEPTH 1
#define MIN_REFLECTION 0.05

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RaytraceRenderer::RaytraceRenderer(uint width, uint height, const vec3& back_color)
: Renderer(width, height, back_color), fs_quad(NULL), use_multithread(true)
{
}

RaytraceRenderer::~RaytraceRenderer()
{
}

// Called when we switch to this renderer
void RaytraceRenderer::setup()
{
	// Setup a fullscreen quad :
	fs_quad = new glutil::Quad(getWidth(), getHeight());

	tri_container.nb_triangles = 0;
	lights = NULL;
	nb_lights = 0;
	max_depth = MAX_DEPTH;
	min_reflection = MIN_REFLECTION;
}

// Called when we switch to another renderer or close the program
void RaytraceRenderer::cleanup()
{
	delete fs_quad;

	delete [] tri_container.triangles;
	tri_container.triangles = NULL;
	tri_container.nb_triangles = 0;
}

// Render one frame :
void RaytraceRenderer::renderArray(Scene* scene)
{
	assert(scene->getCamera() != NULL);
	assert(scene->getElements() != NULL);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Pixel* pixels = fs_quad->getPixels();

	if(use_multithread)
		renderArrayMultithread(pixels, scene->getCamera(), (const ArrayElementContainer*)(scene->getElements()));
	else
		renderArraySinglethread(pixels, scene->getCamera(), (const ArrayElementContainer*)(scene->getElements()));

	fs_quad->markAsUpdated();
	fs_quad->display();
}

// Copy from an ArrayElementContainer to an optimized TriangleContainer while applying tranformations
void RaytraceRenderer::fillTriangleContainerArray(const ArrayElementContainer* elements)
{
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	// Count the number of tri_container of each type
	uint nb_triangles = 0;
	for(uint i=0 ; i < nb_objects ; i++)
	{
		if(objects[i]->getType() == Object::MESH)
			nb_triangles += ((MeshObject*)objects[i])->getGeometry()->getNbVertices() / 3;
	}

	// Allocate memory for the TriangleContainer :
	if(nb_triangles == 0)
	{
		delete [] tri_container.triangles;
		tri_container.triangles = NULL;
	}
	else
	{
		if(tri_container.nb_triangles != nb_triangles)
		{
			delete [] tri_container.triangles;
			tri_container.triangles = new Triangle[nb_triangles];
		}
	}

	tri_container.nb_triangles = nb_triangles;

	// Copy the objects to the TriangleContainer :
	uint num_triangle = 0;
	for(uint i=0 ; i < nb_objects ; i++)
	{
		if(objects[i]->getType() == Object::MESH)
		{
			MeshObject* mesh_obj = (MeshObject*)(objects[i]);
			Geometry* geo = mesh_obj->getGeometry();
			const float* vertices = geo->getVertices();
			const float* normals = geo->getNormals();
			uint nb_triangles_mesh = geo->getNbVertices() / 3;
			Material* material = mesh_obj->getMaterial();
			vec3 position = mesh_obj->getPosition();
			mat3 orientation = mesh_obj->getOrientation();

			uint k=0;
			for(uint j=0 ; j < nb_triangles_mesh ; j++, num_triangle++)
			{
				Triangle& tri = tri_container.triangles[num_triangle];
				tri.v0 = orientation * vec3(vertices[k+0], vertices[k+1], vertices[k+2]) + position;
				tri.v1 = orientation * vec3(vertices[k+3], vertices[k+4], vertices[k+5]) + position;
				tri.v2 = orientation * vec3(vertices[k+6], vertices[k+7], vertices[k+8]) + position;

				tri.normal = orientation * vec3(normals[k+0], normals[k+1], normals[k+2]);

				tri.material = material;

				k += 9;
			}
		}
	}
}

// Render one frame, in case the elements are in an ArrayElementContainer
void RaytraceRenderer::renderArraySinglethread(Pixel* pixels, const Camera* camera, const ArrayElementContainer* elements)
{
	// Get the number of lights and pointer to the lights - those
	// are used later by other member functions
	this->lights = elements->getLights();
	this->nb_lights = elements->getNbLights();

	// Get some values / pointers...etc
	uint width = fs_quad->getWidth();
	uint height = fs_quad->getHeight();

	float fw = float(width-1);
	float fh = float(height-1);
	float fovy_half_rad = 0.5 * (camera->getFOVY() * M_PI / 180.0);
	float tan_fovy_half = tan(fovy_half_rad);

	const mat3& cam_orientation = camera->getOrientation();
	vec3 cam_pos = camera->getPosition();

	// Dimensions of the plane at "z=-1.0" in world space
	float dy = 2.0*tan_fovy_half;
	float dx = dy*camera->getAspect();

	// Copy the tri_container to our cached tri_container container
	fillTriangleContainerArray(elements);

	// For each pixel :
	for(uint x=0 ; x < width ; x++)
	{
		for(uint y=0 ; y < height ; y++)
		{
			Ray r;

			// ----------- STEP 1 : calculate the position and direction of the ray -----------
			// - first, consider the camera is at (0,0,0) with no rotation, pointing towards -Z
			float fx = (float(x) / fw) - 0.5;	// in [-0.5 ; 0.5]
			float fy = (float(y) / fh) - 0.5;	// in [-0.5 ; 0.5]

			vec3 pointed_pos(fx*dx, fy*dy, -1.0);	// position of the corresponding point
													//on the plane at z=-1.0

			// - then, move the ray to the camera's space
			r.start = cam_pos;

			// - calculate the direction of the ray :
			pointed_pos = cam_orientation * pointed_pos + cam_pos;
			r.direction = glm::normalize(pointed_pos - r.start);

			// ----------- STEP 2 : recursively launch a ray -----------
			vec3 final_color = launchColorRay(r);

			// ----------- STEP 3 :Store the computed color in the final image ---------
			Pixel& p = pixels[x + y*width];

			final_color *= 255.0;

			p.r = (uchar)(glm::clamp(final_color.r, 0.0f, 255.0f));
			p.g = (uchar)(glm::clamp(final_color.g, 0.0f, 255.0f));
			p.b = (uchar)(glm::clamp(final_color.b, 0.0f, 255.0f));
		}
	}
}

// Multithreaded version of renderArray() :
// - wrapping for C :
struct MultithreadParams
{
	RaytraceRenderer* that;
	Pixel* pixels;
	const Camera* camera;
	const ArrayElementContainer* elements;
	int thread_id;
	float dx, dy;	// Dimensions of the plane at z=-1.0
};

static void* _renderArrayThreadWrapper(void* p)
{
	MultithreadParams* params = (MultithreadParams*)p;
	params->that->renderArrayThread(params->pixels, params->camera, params->elements,
									params->thread_id, params->dx, params->dy);
	return NULL;
}

// Multithreaded version of renderArray() :
void RaytraceRenderer::renderArrayMultithread(Pixel* pixels, const Camera* camera, const ArrayElementContainer* elements)
{
	// Copy the tri_container to our cached tri_container container
	fillTriangleContainerArray(elements);

	// Get the number of lights and pointer to the lights - those
	// are used later by other member functions
	this->lights = elements->getLights();
	this->nb_lights = elements->getNbLights();

	// Set the starting pixels
	this->index_next_pixel[0] = 0;
	this->index_next_pixel[1] = fs_quad->getWidth() * fs_quad->getHeight() - 1;

	// Compute the dimensions of the plane at "z=-1.0" in world space
	float fovy_half_rad = 0.5 * (camera->getFOVY() * M_PI / 180.0);
	float tan_fovy_half = tan(fovy_half_rad);

	float dy = 2.0*tan_fovy_half;
	float dx = dy*camera->getAspect();

	// Create threads :
	pthread_t inc_thread;
	pthread_t dec_thread;

	MultithreadParams inc_params;
	MultithreadParams dec_params;

	// Initialize the parameters
	// - inc_params
	inc_params.that = this;
	inc_params.camera = camera;
	inc_params.dx = dx;
	inc_params.dy = dy;
	inc_params.elements = elements;
	inc_params.pixels = pixels;
	dec_params.thread_id = 1;

	// - dec_params
	dec_params.that = this;
	dec_params.camera = camera;
	dec_params.dx = dx;
	dec_params.dy = dy;
	dec_params.elements = elements;
	dec_params.pixels = pixels;
	inc_params.thread_id = 0;

	// Launch the threads
	pthread_create(&inc_thread, NULL, &_renderArrayThreadWrapper, &inc_params);
	pthread_create(&dec_thread, NULL, &_renderArrayThreadWrapper, &dec_params);

	// Wait for thread termination
	pthread_join(inc_thread, NULL);
	pthread_join(dec_thread, NULL);
}

// Thread
void RaytraceRenderer::renderArrayThread(Pixel* pixels, const Camera* camera,
										const ArrayElementContainer* elements,
										int thread_id, float dx, float dy)
{
	uint width = fs_quad->getWidth();
	uint height = fs_quad->getHeight();
	int index_max = int(width*height - 1);

	float fw = float(width-1);
	float fh = float(height-1);

	const vec3& cam_pos = camera->getPosition();
	const mat3& cam_orientation = camera->getOrientation();

	// Thread 0 : increment
	if(thread_id == 0)
	{
		int& i = index_next_pixel[0];
		for(i=0 ; i <= index_next_pixel[1] && i <= index_max ; i++)
		{
			Ray r;

			int y = i / width;
			int x = i - y*width;

			// Calculate the position and direction of the ray
			// - first, consider the camera is at (0,0,0) with no rotation, pointing towards -Z
			float fx = (float(x) / fw) - 0.5f;	// in [-0.5 ; 0.5]
			float fy = (float(y) / fh) - 0.5f;	// in [-0.5 ; 0.5]

			vec3 pointed_pos(fx*dx, fy*dy, -1.0);	// position of the corresponding point
													//on the plane at z=-1.0

			// - then, move the ray to the camera's space
			r.start = cam_pos;

			// - calculate the direction of the ray :
			pointed_pos = cam_orientation * pointed_pos + cam_pos;
			r.direction = glm::normalize(pointed_pos - r.start);

			// Launch a ray and get the color for the pixel
			vec3 final_color = launchColorRay(r) * 255.0f;

			// Store the computed color in the final image
			Pixel& p = pixels[i];

			p.r = (uchar)(glm::clamp(final_color.r, 0.0f, 255.0f));
			p.g = (uchar)(glm::clamp(final_color.g, 0.0f, 255.0f));
			p.b = (uchar)(glm::clamp(final_color.b, 0.0f, 255.0f));
		}
	}
	// Thread 1 : decrement
	else if(thread_id == 1)
	{
		int& i = index_next_pixel[1];
		for(i=index_max ; i >= index_next_pixel[0] && i >= 0 ; i--)
		{
			// Same thing as above...
			Ray r;

			int y = i / width;
			int x = i - y*width;

			float fx = (float(x) / fw) - 0.5;
			float fy = (float(y) / fh) - 0.5;

			vec3 pointed_pos(fx*dx, fy*dy, -1.0);

			r.start = cam_pos;

			pointed_pos = cam_orientation * pointed_pos + cam_pos;
			r.direction = glm::normalize(pointed_pos - r.start);

			vec3 final_color = launchColorRay(r) * 255.0f;

			Pixel& p = pixels[i];

			p.r = (uchar)(glm::clamp(final_color.r, 0.0f, 255.0f));
			p.g = (uchar)(glm::clamp(final_color.g, 0.0f, 255.0f));
			p.b = (uchar)(glm::clamp(final_color.b, 0.0f, 255.0f));
		}
	}
}

// Launching one ray and get the distance to the intersection
RaytraceRenderer::Triangle* RaytraceRenderer::launchRay(const Ray& r, float* pt) const
{
	float t = -1.0;
	float t_min = -1.0;
	Triangle* closest_triangle = NULL;

	// Find the closest triangle
	for(uint i=0 ; i < tri_container.nb_triangles ; i++)
	{
		Triangle& tri = tri_container.triangles[i];
		if(rayHitsTriangle(r, tri.v0, tri.v1, tri.v2, &t))
		{
			// Only remember it if it's the closest intersection we ever had
			if(t_min < 0.0 || t < t_min)
			{
				t_min = t;
				closest_triangle = &tri;
			}
		}
	}

	*pt = t_min;

	return closest_triangle;
}

// Recursive launching of rays resulting in a color value
vec3 RaytraceRenderer::launchColorRay(const Ray& r, uint depth) const
{
	float t = -1.0;
	Triangle* tri= NULL;

	// If there is no intersection, return the background color
	tri = launchRay(r, &t);
	if(tri == NULL)
		return getBackColor();
	else	// If there is an intersection :
	{
		vec3 pos = r.start + t*r.direction;

		// Compute the resulting color :
		vec3 final_color;
		const RaytraceProfile* mat_profile = (const RaytraceProfile*)(tri->material->getProfile(RAYTRACE_PROFILE));

		// Add the emissive term
		final_color = mat_profile->getEmissive();

		// For each light in the scene, if it is not occluded, add the diffuse component
		for(uint i=0 ; i < nb_lights ; i++)
		{
			Light* l = lights[i];
			vec3 light_vec = l->getPosition() - pos;
			float dist_light = glm::length(light_vec);
			light_vec /= dist_light;

			// Test if the surface faces the light :
			float dot_product = glm::dot(light_vec, tri->normal);
			if(dot_product < 0.0)
				continue;

			// Test if it is in shadow :
			if(launchRay(Ray(pos, light_vec), &t) == NULL ||	// no intersection
			   t > dist_light)	// intersection with an object too far
			{
				// Not in shadow => add the diffuse contribution of the light
				final_color += mat_profile->getDiffuse() * dot_product;
			}

			// Add reflection :
			if(depth < max_depth)
			{
				Ray reflected_ray;
				reflected_ray.start = pos;
				reflected_ray.direction = glm::normalize(2.0f*tri->normal - r.direction);
				final_color += mat_profile->getReflection() * launchColorRay(reflected_ray, depth+1);
			}
		}
		return final_color;
	}
}

// Ray / triangle intersection code
inline bool
RaytraceRenderer::rayHitsTriangle(const Ray& r, const vec3& v0, const vec3& v1, const vec3& v2, float* t) const
{
	vec3 e1, e2, h, s, q;
	float a, f, u, v;

	e1 = v1 - v0;
	e2 = v2 - v0;

	h = glm::cross(r.direction, e2);
	a = glm::dot(e1, h);

	if(a > -0.00001 && a < 0.00001)
		return false;

	f = 1/a;

	s = r.start - v0;

	u = f * glm::dot(s, h);

	if(u < 0.0 || u > 1.0)
		return false;

	q = glm::cross(s, e1);

	v = f * glm::dot(r.direction, q);
	if(v < 0.0 || u + v > 1.0)
		return false;

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	*t = f * glm::dot(e2, q);
	if(*t > 0.00001)	// ray intersection
		return true;
	else	// this means that there is a line intersection, but not a ray intersection
		return false;
}

// Implementation of KeyEventReceiver :
void RaytraceRenderer::onKeyEvent(int key, int action)
{
	if(action == GLFW_RELEASE)
	{
		if(key == 'M')
		{
			use_multithread = !use_multithread;
			if(use_multithread)
				cout << "Start using multithread" << endl;
			else
				cout << "Stop using multithread" << endl;
		}
	}
}
