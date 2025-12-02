#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2|resit)

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "vec3.hpp"
#include "vec4.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator [] allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m[1,2];
 *    m[0,3] = 3.f;
 *
 * (Multi-dimensionsal subscripts in operator[] is a C++23 feature!)
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator[] (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator[] (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr
Mat44f operator*( Mat44f const& aLeft, Mat44f const& aRight ) noexcept
{
	Mat44f R = { {
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f
	} };

	//loop over each element and perform the row/column opperations
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				R[i, j] = R[i, j] + (aLeft[i, k] * aRight[k, j]);
			}
		}
	}

	return R;
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	float x = (aLeft[0, 0] * aRight[0]) + (aLeft[0, 1] * aRight[1]) + (aLeft[0, 2] * aRight[2]) + (aLeft[0, 3] * aRight[3]);
	float y = (aLeft[1, 0] * aRight[0]) + (aLeft[1, 1] * aRight[1]) + (aLeft[1, 2] * aRight[2]) + (aLeft[1, 3] * aRight[3]);
	float z = (aLeft[2, 0] * aRight[0]) + (aLeft[2, 1] * aRight[1]) + (aLeft[2, 2] * aRight[2]) + (aLeft[2, 3] * aRight[3]);
	float w = (aLeft[3, 0] * aRight[0]) + (aLeft[3, 1] * aRight[1]) + (aLeft[3, 2] * aRight[2]) + (aLeft[3, 3] * aRight[3]);

	return { x, y, z, w };
}

// Functions:

Mat44f invert( Mat44f const& aM ) noexcept;

inline
Mat44f transpose( Mat44f const& aM ) noexcept
{
	Mat44f ret;
	for( std::size_t i = 0; i < 4; ++i )
	{
		for( std::size_t j = 0; j < 4; ++j )
			ret[j,i] = aM[i,j];
	}
	return ret;
}

inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	Mat44f R = kIdentity44f;

	R[1, 1] = cos(aAngle);
	R[1, 2] = -sin(aAngle);
	R[2, 1] = sin(aAngle);
	R[2, 2] = cos(aAngle);

	return R;
}


inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	Mat44f R = kIdentity44f;

	R[0, 0] = cos(aAngle);
	R[0, 2] = sin(aAngle);
	R[2, 0] = -sin(aAngle);
	R[2, 2] = cos(aAngle);

	return R;
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	Mat44f R = kIdentity44f;

	R[0, 0] = cos(aAngle);
	R[0, 1] = -sin(aAngle);
	R[1, 0] = sin(aAngle);
	R[1, 1] = cos(aAngle);

	return R;
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	Mat44f R = kIdentity44f;

	R[3, 0] = aTranslation[0];
	R[3, 1] = aTranslation[1];
	R[3, 2] = aTranslation[2];

	return R;
}
inline
Mat44f make_scaling( float aSX, float aSY, float aSZ ) noexcept
{
	Mat44f R = kIdentity44f;

	R[0, 0] = aSX;
	R[1, 1] = aSY;
	R[2, 2] = aSZ;

	return R;
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	const float s { 1.f / static_cast<float>(tan( aFovInRadians / 2.f )) };
	const float sx{ s / aAspect };
	const float sy{ s };

	const float a{ -((aFar + aNear) / (aFar - aNear)) };
	const float b{ -2 * ((aFar * aNear) / (aFar - aNear)) };

	return {
		sx,  0,  0,  0,
		 0, sy,  0,  0,
		 0,  0,  a,  b,
		 0,  0, -1,  0,
	};
}

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
