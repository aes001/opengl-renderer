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

/*Mat44f MakeBillboardLookAt(Mat44f LookAt)
{
	Mat33f Look_At_upper = mat44_to_mat33(LookAt);
	Mat33f I = transpose(Look_At_upper);
	Mat44f R = kIdentity44f;
	R[0, 0] = I[0, 0]; R[0, 1] = I[0, 1]; R[0, 2] = I[0, 2];
	R[1, 0] = I[1, 0]; R[1, 1] = I[1, 1]; R[1, 2] = I[1, 2];
	R[2, 0] = I[2, 0]; R[2, 1] = I[2, 1]; R[2, 2] = I[2, 2];

	return R;

}*/

Mat44f MakeBillboardLookAt(const Vec3f& D, const Vec3f& U, const Vec3f& R)
{
	/*Mat44f Axis = { R[0], U[0], 0, 0,
				   R[1], U[1], 0, 0,
				   R[2], U[2], 1, 0,
				   0,    0,    0,    1 };*/

	Mat33f Axis = { R[0], R[1], R[2],
				   U[0], U[1], U[2], 
				   D[0], D[1], D[2]};
	Mat33f I = inverse(Axis);

	Mat44f ret = kIdentity44f;
	ret[0, 0] = I[0, 0]; ret[0, 1] = I[0, 1]; ret[0, 2] = I[0, 2];
	ret[1, 0] = I[1, 0]; ret[1, 1] = I[1, 1]; ret[1, 2] = I[1, 2];
	ret[2, 0] = I[2, 0]; ret[2, 1] = I[2, 1]; ret[2, 2] = I[2, 2];

	return ret;

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