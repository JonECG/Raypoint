#pragma once

#include <functional>
#include "Camera.h"

class RayMarcher
{
	Camera * camera;

public:
	std::function<bool(float,float,float,float)> targetFunction;
	RayMarcher(std::function<bool(float,float,float,float)> targetFunction, Camera * camera);

	bool marchRay( float x, float y, float * distance, glm::vec3 * normal );
};

