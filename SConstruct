# SConstruct
# - Cross-compilation: scons crossmingw=1
# - OpenCL and GL3W support has only been done and tested on Linux.
# - GL3W can only be used with GLFW SVN version, before gl3.h support has
# removed! (tested with revision r1202)

import sys

# ---------------
# Options:
USE_OPENCL = False
USE_GL3W = False	# Do we use gl3w (and gl3.h) or GLEW (and gl.h) ?

use_mingw = False	# scons crossmingw=1 to set to True on Linux
# ---------------

# If we use gl3w: compile it
if USE_GL3W:
	SConscript('gl3w/SConstruct')

# ----------------------------------------------------------------------
VariantDir('obj', 'src', duplicate=0)

def replace_src_obj(src_list):
	for i in range(0, len(src_list)):
		src_list[i] = src_list[i].replace('src', 'obj')

# Main sources :
src_list = Split("""
src/Application.cpp
src/animators/CameraAnimator.cpp
src/animators/FPSCameraAnimator.cpp
src/animators/KeyboardCameraAnimator.cpp
src/animators/SceneAnimator.cpp
src/animators/LightSceneAnimator.cpp
src/glutil/glutil.cpp
src/glutil/GPUProgram.cpp
src/glutil/Quad.cpp
src/glutil/TextureCreation.cpp
src/gui/GLFWWindow.cpp
src/log/Log.cpp
src/log/LogHTML.cpp
src/log/LogRTF.cpp
src/log/LogTermVT100.cpp
src/log/LogTermWindows.cpp
src/preprocessor/ExpressionEvaluator.cpp
src/preprocessor/Preprocessor.cpp
src/renderer/Renderer.cpp
src/renderer/DebugRenderer.cpp
src/renderer/DeferredShadingRenderer.cpp
src/renderer/DepthPeelingRenderer.cpp
src/renderer/MultiLayerRenderer.cpp
src/renderer/MyRenderer.cpp
src/renderer/MyRenderer2.cpp
src/renderer/RasterRenderer.cpp
src/renderer/RaytraceRenderer.cpp
src/renderer/StencilRoutedRenderer.cpp
src/renderer/utils/BounceMap.cpp
src/renderer/utils/GBuffer.cpp
src/renderer/utils/GBufferRenderer.cpp
src/renderer/utils/GLRaytracer.cpp
src/renderer/utils/MinMaxMipmaps.cpp
src/renderer/utils/PhotonsMap.cpp
src/renderer/utils/PhotonVolumesRenderer.cpp
src/renderer/utils/ShadowMap.cpp
src/renderer/utils/TextureBinding.cpp
src/renderer/utils/TexunitManager.cpp
src/renderer/utils/TextureReducer.cpp
src/scene/Camera.cpp
src/scene/DAELoader.cpp
src/scene/Element.cpp
src/scene/ElementContainer.cpp
src/scene/ArrayElementContainer.cpp
src/scene/Geometry.cpp
src/scene/GPUProgramManager.cpp
src/scene/Light.cpp
src/scene/Material.cpp
src/scene/MeshObject.cpp
src/scene/Object.cpp
src/scene/OBJLoader.cpp
src/scene/Scene.cpp
src/scene/SceneLoader.cpp
src/scene/Sphere.cpp
src/scene/profiles/Profile.cpp
src/scene/profiles/DepthPeelingProfile.cpp
src/scene/profiles/GeneralProfile.cpp
src/scene/profiles/GPUProfile.cpp
src/scene/profiles/RaytraceProfile.cpp
src/tinyxml/tinyxml.cpp
src/tinyxml/tinyxmlerror.cpp
src/tinyxml/tinyxmlparser.cpp
src/utils/TGALoader.cpp
src/utils/XMLManip.cpp
""")

# Create the environment
env = Environment()

# If we are on Windows or cross-compiling, use the appropriate tool
# and set "use_mingw" to True
use_mingw = False
if sys.platform == 'win32':
	use_mingw = True
	Tool('mingw')(env)
else:
	if ARGUMENTS.get('crossmingw', 0):
		use_mingw = True
		env.Tool('crossmingw', toolpath = ['scons-tools'])

# Setup MinGW:
if use_mingw:
	env.Append(LIBPATH=['#/mingw/lib'])
	env.Append(CPPPATH=['#/mingw/include'])

# OpenCL:
if USE_OPENCL:
	cl_src_list = Split("""
		src/clutil/clutil.cpp
		src/renderer/utils/OCLRaytracer.cpp
		""")
	src_list += cl_src_list

	env.Append(LIBS=['OpenCL'])
	env.Append(CPPDEFINES=['USE_OPENCL'])

# gl3w:
if USE_GL3W:
	env.Append(CPPDEFINES=['USE_GL3W'])
	env.Append(CPPPATH=['#/gl3w/include'])
	env.Append(LIBPATH=['#/gl3w'])
	env.Append(CPPDEFINES=['GLFW_INCLUDE_GL3'])
	env.Append(LIBS=['gl3w'])
# GLEW:
else:
	if use_mingw:
		env.Append(CPPDEFINES=['GLEW_STATIC'])
		env.Append(LIBS=['glew32s'])
	else:
		env.ParseConfig('pkg-config glew --libs --cflags')

# GLFW:
if use_mingw:
	env.Append(LIBS=['glfw'])
else:
	env.ParseConfig('pkg-config libglfw --libs --cflags')

# OpenGL
if use_mingw:
	env.Append(LIBS=['opengl32'])

# pthread:
if use_mingw:
	env.Append(LIBS=['pthreadGC2'])
else:
	env.Append(LIBS=['pthread'])

env.Append(CPPDEFINES=['TIXML_USE_STL'])
env.Append(CCFLAGS=['-g', '-Wall'])	# -O2
env.Append(CCFLAGS=['-s', '-Wall'])	# -O2

# ----------------------------------------------------------------------
replace_src_obj(src_list)

src_obj = env.Object(src_list)

env.Program('renderer', [src_obj, 'obj/main.cpp'])

Export('env')
Export('src_obj')
SConscript('tests/SConstruct')
