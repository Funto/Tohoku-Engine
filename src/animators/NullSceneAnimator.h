// NullSceneAnimator.h

#ifndef NULL_SCENE_ANIMATOR_H
#define NULL_SCENE_ANIMATOR_H

#include "SceneAnimator.h"

class NullSceneAnimator : public SceneAnimator
{
public:
	NullSceneAnimator(Scene* scene = NULL) : SceneAnimator(scene)
	{
	}

	virtual ~NullSceneAnimator()
	{
	}

	virtual void update(double elapsed)
	{
	}

	virtual const char* getName() const {return "NullSceneAnimator";}

	virtual void printHelp() const {}
};

#endif // NULL_SCENE_ANIMATOR_H
