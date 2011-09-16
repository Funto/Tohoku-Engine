// StencilRoutedRenderer.cpp

#include "StencilRoutedRenderer.h"

#define ATTRIB_POSITION 0
#define FRAG_DATA_COLOR 0

StencilRoutedRenderer::StencilRoutedRenderer(uint width, uint height)
: Renderer(width, height, vec3(0.0, 0.0, 0.0)),
  write_fragments_program(NULL),
  sort_fragments_program(NULL)
{
}

StencilRoutedRenderer::~StencilRoutedRenderer()
{
}

// Called when we switch to this renderer
void StencilRoutedRenderer::setup()
{
	createPrograms();
}

// Called when we switch to another renderer or close the program
void StencilRoutedRenderer::cleanup()
{
	delete write_fragments_program;
	write_fragments_program = NULL;

	delete sort_fragments_program;
	sort_fragments_program = NULL;
}

// Called when we change the scene
void StencilRoutedRenderer::loadSceneArray(Scene* scene)
{
}

void StencilRoutedRenderer::unloadSceneArray(Scene* scene)
{
}

// Render a frame
void StencilRoutedRenderer::renderArray(Scene* scene)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

// Debug drawing for the renderers
void StencilRoutedRenderer::debugDraw2D(Scene* scene)
{
}

void StencilRoutedRenderer::debugDraw3D(Scene* scene)
{
}

void StencilRoutedRenderer::createPrograms()
{
	glutil::GPUProgram* p = NULL;
	bool ok = false;

	// --------  write_fragments_program ---------
	p = new glutil::GPUProgram("media/shaders/stencil_routing/write_fragments.vert",
							   "media/shaders/stencil_routing/write_fragments.frag");

	// Here: preprocessor symbols

	ok = p->compileAndAttach();
	assert(ok);

	p->bindAttribLocation(ATTRIB_POSITION, "vertex_position");
	p->bindFragDataLocation(FRAG_DATA_COLOR, "frag_color");
	ok &= p->link();
	assert(ok);

	// Here: set uniforms' names

	p->validate();

	write_fragments_program = p;

	// --------  sort_fragments_program ---------
	p = new glutil::GPUProgram("media/shaders/stencil_routing/sort_fragments.vert",
							   "media/shaders/stencil_routing/sort_fragments.frag");

	// Here: preprocessor symbols

	ok = p->compileAndAttach();
	assert(ok);

	p->bindAttribLocation(ATTRIB_POSITION, "vertex_position");
	p->bindFragDataLocation(FRAG_DATA_COLOR, "frag_color");
	ok &= p->link();
	assert(ok);

	// Here: set uniforms' names

	p->validate();

	sort_fragments_program = p;
}
