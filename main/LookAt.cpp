#include "LookAt.hpp"



//P = Postion, D = Direction, U = Up, R = Right
Mat44f MakeLookAt( const Vec3f& P, const Vec3f& D, const Vec3f& U, const Vec3f& R )
{
	Mat44f Axis = { R[0], R[1], R[2], 0,
				   U[0], U[1], U[2], 0,
				   D[0], D[1], D[2], 0,
				   0,    0,    0,    1 };
	Mat44f translation = make_translation(P);

	return Axis * translation;
}
