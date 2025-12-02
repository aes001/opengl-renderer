// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.
#define _USE_MATH_DEFINES
#include <catch2/catch_amalgamated.hpp>

#include <numbers>
#include <math.h>

#include "../vmlib/mat44.hpp"

TEST_CASE("Rotation-x", "[mat44]")
{
	static constexpr float kEps_ = 1e-4f;

	using namespace Catch::Matchers;

	SECTION("30 degree")
	{
		Mat44f expected = { {
			1.000000f, 0.000000f, 0.000000f, 0.000000f,
			0.000000f, 0.8660254f, -0.500000f, 0.000000f,
			0.000000f, 0.500000f, 0.8660254f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f
		} };

		Mat44f result = make_rotation_x(M_PI / 6); // 30-degree rotation
		
		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

	SECTION("small rotation")
	{
		Mat44f expected = { {
			1.000000f, 0.000000f, 0.000000f, 0.000000f,
			0.000000f, 0.99950656f, -0.03141076f, 0.000000f,
			0.000000f, 0.03141076f, 0.99950656f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f
		} };


		Mat44f result = make_rotation_x(M_PI / 100); // 1.8-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

	SECTION("negative rotation") 
	{
		Mat44f expected = { {
			1.000000f, 0.000000f, 0.000000f, 0.000000f,
			0.000000f, 0.70710678f, 0.70710678f, 0.000000f,
			0.000000f, -0.70710678f, 0.70710678f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f
		} };


		Mat44f result = make_rotation_x(- M_PI / 4); // -45-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

}

TEST_CASE("Rotation-y", "[mat44]")
{
	static constexpr float kEps_ = 1e-4f;

	using namespace Catch::Matchers;

	SECTION("30 degree")
	{
		Mat44f expected = { {
			0.8660254f, 0.000000f, 0.500000f, 0.000000f,
			0.000000f, 1.000000f, 0.000000f, 0.000000f,
			-0.500000f, 0.000000f, 0.8660254f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f

		} };

		Mat44f result = make_rotation_y(M_PI / 6); // 30-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

	SECTION("small rotation")
	{
		Mat44f expected = { {
			0.99950656f, 0.000000f, 0.03141076f, 0.000000f,
			0.000000f, 1.000000f, 0.000000f, 0.000000f,
			-0.03141076f, 0.000000f, 0.99950656f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f

		} };


		Mat44f result = make_rotation_y(M_PI / 100); // 1.8-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

	SECTION("negative rotation")
	{
		Mat44f expected = { {
			0.70710678f, 0.000000f, -0.70710678f, 0.000000f,
			0.000000f, 1.000000f, 0.000000f, 0.000000f,
			0.70710678f, 0.000000f, 0.70710678f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f

		} };


		Mat44f result = make_rotation_y(-M_PI / 4); // -45-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

}

TEST_CASE("Rotation-z", "[mat44]")
{
	static constexpr float kEps_ = 1e-4f;

	using namespace Catch::Matchers;

	SECTION("30 degree")
	{
		Mat44f expected = { {
			0.8660254f, -0.500000f, 0.000000f, 0.000000f,
			0.500000f, 0.8660254f, 0.000000f, 0.000000f,
			0.000000f, 0.000000f, 1.000000f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f

		} };

		Mat44f result = make_rotation_z(M_PI / 6); // 30-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

	SECTION("small rotation")
	{
		Mat44f expected = { {
			0.99950656f, -0.03141076f, 0.000000f, 0.000000f,
			0.03141076f, 0.99950656f, 0.000000f, 0.000000f,
			0.000000f, 0.000000f, 1.000000f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f

		} };


		Mat44f result = make_rotation_z(M_PI / 100); // 1.8-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

	SECTION("negative rotation")
	{
		Mat44f expected = { {
			0.70710678f, 0.70710678f, 0.000000f, 0.000000f,
			-0.70710678f, 0.70710678f, 0.000000f, 0.000000f,
			0.000000f, 0.000000f, 1.000000f, 0.000000f,
			0.000000f, 0.000000f, 0.000000f, 1.000000f
		} };


		Mat44f result = make_rotation_z(-M_PI / 4); // -45-degree rotation

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected[i, j], kEps_));
			}
		}
	}

}