#ifndef LIGHTS_H
#define LIGHTS_H

// Includes
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "glad/glad.h"

// Standard Library Includes
#include <vector>
#include <string>

struct Vec3f;

struct DirLight {

	Vec3f direction;

	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
};


#endif