// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.

#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"

TEST_CASE("translation", "[mat44]")
{
	static constexpr float kEps_ = 1e-4f;

	using namespace Catch::Matchers;

	SECTION("diagonal")
	{
		Mat44f test_m = { {
			1.f, 0.f, 0.f, 1.f,
			0.f, 1.f, 0.f, 1.f,
			0.f, 0.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 1.f
		} };

		Mat44f result = make_translation({ 1.f, 1.f, 1.f });

		//ensure no change has been made to test_m
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(test_m[i, j], kEps_));
			}
		}
	}

	SECTION("small")
	{
		Mat44f test_m = { {
			1.f, 0.f, 0.f, 0.000001f,
			0.f, 1.f, 0.f, 0.000001f,
			0.f, 0.f, 1.f, 0.000001f,
			0.f, 0.f, 0.f, 1.f
		} };

		Mat44f result = make_translation({ 0.000001f, 0.000001f, 0.000001f });

		//ensure no change has been made to test_m
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(test_m[i, j], kEps_));
			}
		}
	}
}