#!/bin/bash

# C++
pushd src > /dev/null

NB_MY_CPP_LINES=`wc -l *.cpp *.h	\
		animators/*.cpp	animators/*.h	\
		clutil/*.cpp	clutil/*.h		\
		glutil/*.cpp	glutil/*.h		\
		gui/*.cpp	gui/*.h				\
		log/*.cpp	log/*.h				\
		preprocessor/*.cpp	preprocessor/*.h	\
		renderer/*.cpp	renderer/*.h			\
		renderer/*/*.cpp	renderer/*/*.h		\
		scene/*.cpp	scene/*.h					\
		scene/*/*.cpp	scene/*/*.h				\
		utils/*.cpp	utils/*.h	utils/*.hpp | tail -n 1 | awk '{print $1}'`

NB_LIB_CPP_LINES=`wc -l tinyxml/*.cpp	tinyxml/*.h		\
			glm/*.*							\
			glm/*/*.* | tail -n 1 | awk '{print $1}'`

popd > /dev/null

# GLSL
pushd media/shaders > /dev/null

NB_SHADER_LINES=`wc -l *.vert			*.frag			*.shader	\
				debug/*.vert	debug/*.frag				\
				library/*.shader							\
				raytracer/*.vert	raytracer/*.frag | tail -n 1 | awk '{print $1}'`
popd > /dev/null

# Print result
echo "Lines C++ written by me: " $NB_MY_CPP_LINES
echo "Lines C++ from the libraries: " $NB_LIB_CPP_LINES
echo "Total C++ lines: " `expr $NB_MY_CPP_LINES + $NB_LIB_CPP_LINES`
echo "Lines GLSL: " $NB_SHADER_LINES
echo "Total number of lines written by me: " `expr $NB_MY_CPP_LINES + $NB_SHADER_LINES`
echo "Total number of lines: " `expr $NB_MY_CPP_LINES + $NB_LIB_CPP_LINES + $NB_SHADER_LINES`
