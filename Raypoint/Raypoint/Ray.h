#pragma once

#include "glm\glm.hpp"

struct Ray
{
	glm::vec3 position, direction;
public:
	Ray( glm::vec3 position, glm::vec3 direction );
};

