#ifndef LOOK_AT_H
#define LOOK_AT_H





#include "../vmlib/mat44.hpp"
#include "../vmlib/vec3.hpp"




Mat44f MakeLookAt(const Vec3f& P, const Vec3f& D, const Vec3f& U, const Vec3f& R);

Mat44f MakeOrthoProj(int const left, int const right, int const bottom, int const top, int const zNear, int const zFar);



#endif // LOOK_AT_H