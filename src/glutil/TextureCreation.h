// TextureCreation.h

#ifndef TEXTURE_CREATION_H
#define TEXTURE_CREATION_H

#include "glxw.h"
#include "../Common.h"

namespace glutil
{

// 2D, depth
GLuint createTextureDepth(uint width, uint height);

// Rect, depth
GLuint createTextureRectDepth(uint width, uint height);

// 2D, RGBA8
GLuint createTextureRGBA8(uint width, uint height, const GLubyte* data=NULL);

// Rect, RGBA8
GLuint createTextureRectRGBA8(uint width, uint height, const GLubyte* data=NULL);

// 2D, RED8
GLuint createTextureR8(uint width, uint height, const GLubyte* data=NULL);

// Rect, RED8
GLuint createTextureRectR8(uint width, uint height, const GLubyte* data=NULL);

// 2D, RGBA16F|RGBA32F
GLuint createTextureRGBAF(uint width, uint height, bool use_half_float);

// Rect, RGBA16F|RGBA32F
GLuint createTextureRectRGBAF(uint width, uint height, bool use_half_float);

// Renderbuffer, RGBA8
GLuint createRenderbufferRGBA8(uint width, uint height);

// Renderbuffer, depth
GLuint createRenderbufferDepth(uint width, uint height);

}

#endif // TEXTURE_CREATION_H
