// Config.h

#ifndef CONFIG_H
#define CONFIG_H

// Uncomment (resp. comment) to add (or not) the CPU raytracer to the list of
// available renderers
#ifdef USE_OPENCL	// defined or not in the SConstruct
//#define CREATE_CPU_RAYTRACER
#endif

// 0: Debug renderer
// 1: Forward shading renderer
// 2: Deferred shading renderer
// 3: Depth peeling renderer (DEPRECATED)
// 4: Stencil-routed A-Buffer
// 5: Multi layer renderer (deferred shading + depth peeling)
// 6: My own technique
// 7: CPU Raytracer
#define DEFAULT_RENDERER 6

// 0: Cornell Suzanne
// 1: Cube
// 2: Cube & wall
// 3: Balls
// 4: Caustics
// 5: Sponza atrium (TODO)
#define DEFAULT_SCENE 0

// 0: KeyboardCameraAnimator
// 1: FPSCameraAnimator
#define DEFAULT_CAMERA_ANIMATOR 1

// 0: NullSceneAnimator
// 1: LightSceneAnimator
#define DEFAULT_SCENE_ANIMATOR 1

// Should we use shadow mapping?
#define USE_SHADOW_MAPPING true

// In case we enabled shadow mapping, should we use visibility maps
// for deferred shading, or compute the shadows directly while rendering
// the full-screen quad?
#define USE_VISIBILITY_MAPS false

// Enable vertical synchronization?
#define ENABLE_VSYNC true

// Name of the BRDF function to use:
//#define BRDF_FUNCTION "blinn_phong"
#define BRDF_FUNCTION "ashikhmin_shirley"

// Filename of the kernel to use:
#define KERNEL_FILENAME "media/textures/fading.tga"
//#define KERNEL_FILENAME "media/textures/gaussian.tga"

// Window configuration:
#define WIN_TITLE "3D renderer"
//#define WIN_WIDTH 640
//#define WIN_HEIGHT 480
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Camera settings
#define OVERRIDE_CAMERA_SETTINGS	// comment for keeping the values given by the COLLADA file
#ifdef OVERRIDE_CAMERA_SETTINGS
	#define CAMERA_FOVY 49.13434
	#define CAMERA_ASPECT (float(WIN_WIDTH)/float(WIN_HEIGHT))
	//#define CAMERA_ZNEAR 1.0
	#define CAMERA_ZNEAR 0.5
	#define CAMERA_ZFAR 100.0
	//#define CAMERA_ZFAR 5.0
	//#define CAMERA_ZFAR 10.0
#endif

// Background color
#define BACK_COLOR vec3(0.4, 0.4, 0.7)

// Number of depth layers for the depth peeling:
#define NB_DEPTH_LAYERS 4

#endif // CONFIG_H
