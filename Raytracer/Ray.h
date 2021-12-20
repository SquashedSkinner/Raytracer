#pragma once
#include "glm/glm.hpp"

class Ray
{
	// This class defines the ray which will be used in
	// the raytracing calculations
	public:	
	glm::vec3 origin;
	glm::vec3 direction;
	
	Ray(const glm::vec3 _orig, const glm::vec3 _dir)
	{
		origin = _orig; direction = _dir; 
	}
	glm::vec3 point(float t) const
	{
		return origin + t * direction;
	}

};