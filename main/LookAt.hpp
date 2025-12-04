#ifndef LOOK_AT_H
#define LOOK_AT_H





#include "../vmlib/mat44.hpp"
#include "../vmlib/vec3.hpp"




Mat44f MakeLookAt(const Vec3f& P, const Vec3f& D, const Vec3f& U, const Vec3f& R);





#endif // LOOK_AT_H