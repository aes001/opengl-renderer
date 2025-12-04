#ifndef LOOK_AT_H
#define LOOK_AT_H





#include "../vmlib/mat44.hpp"
#include "../vmlib/vec3.hpp"




Mat44f MakeLookAt( const Vec3f& pos, const Vec3f& target, const Vec3f& up );





#endif // LOOK_AT_H