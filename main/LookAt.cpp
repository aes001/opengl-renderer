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

Mat44f MakeOrthoProj(int const left, int const right, int const bottom, int const top, int const zNear, int const zFar )
{
	Mat44f Result;
	Result[0, 0] = float(2 / (right - left));
	Result[1, 1] = float(2 / (top - bottom));
	Result[2, 2] = float(-2 / (zFar - zNear));
	Result[3, 0] = float(-(right + left) / (right - left));
	Result[3, 1] = float(-(top + bottom) / (top - bottom));
	Result[3, 2] = float(-(zFar + zNear) / (zFar - zNear));

	return Result;
}