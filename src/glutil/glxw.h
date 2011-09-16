// glxw.h

#ifndef GLXW_H
#define GLXW_H

#include "../log/Log.h"

#ifdef USE_GL3W
	#include <GL3/gl3w.h>
	inline bool glxwInit()
	{
		if(gl3wInit() != 0)
		{
			logError("error initializing gl3w");
			return false;
		}
		return true;
	}
#else
	#ifdef __cplusplus
	extern "C" {
	#endif

		#include <GL/glew.h>

	#ifdef __cplusplus
	}
	#endif

	inline bool glxwInit()
	{
		// GLEW is buggy: it uses glGetString(GL_EXTENSIONS)
		// instead of glGetStringi(), so we need to set glewExperimental to GL_TRUE
		// and get the first error...
		// See: http://sourceforge.net/tracker/?func=detail&aid=2927731&group_id=67586&atid=523274
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if(GLEW_OK != err)
		{
			logError("error initializing GLEW: ", glewGetErrorString(err));
			return false;
		}
		glGetError();
		return true;
	}

#endif

#endif // GLXW_H
