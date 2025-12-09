#ifndef GEOMETRIC_HELPERS_HPP
#define GEOMETRIC_HELPERS_HPP

#include <numbers>
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"


constexpr
float operator"" _deg( long double aDegAngle )
{
	return static_cast<float>(aDegAngle * (std::numbers::pi_v<long double> / 180.L));
}


constexpr
float operator"" _rad( long double aRadAngle )
{
	return static_cast<float>(aRadAngle);
}


inline constexpr
float ToRad( float aDegAngle )
{
	return aDegAngle * (std::numbers::pi_v<float> / 180.f);
}


inline constexpr
float ToDeg( float aRadAngle )
{
	return aRadAngle * (180.0f / std::numbers::pi_v<float>);
}


inline constexpr
Vec3f Vec4ToVec3 (const Vec4f& v)
{
	return Vec3f{v.x, v.y, v.z};
}


inline constexpr
Vec4f Vec3ToVec4 (const Vec3f& v)
{
	return Vec4f{v.x, v.y, v.z, 0.f};
}


#endif // GEOMETRIC_HELPERS_HPP