// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.
#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"

TEST_CASE("4x4 matrix multiplication", "[mat44]")
{
	static constexpr float kEps_ = 1e-4f;

	using namespace Catch::Matchers;

	SECTION("Identity") 
	{
		Mat44f test_m = { {
			0.f, 1.f, 2.f, 3.f,
			1.f, 2.f, 3.f, 4.f,
			2.f, 3.f, 4.f, 5.f,
			3.f, 4.f, 5.f, 6.f
		} };

		Mat44f result = kIdentity44f * test_m;

		//ensure no change has been made to test_m
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(test_m[i, j], kEps_));
			}
		}
		

	}

	SECTION("Random")
	{
		Mat44f test_m_1 = { {
			1.5f, 7.2f, 0.7f, 8.1f,
			0.9f, 1.5f, 4.2f, 5.8f,
			1.1f, 9.4f, 6.3f, 0.8f,
			5.9f, 4.1f, 7.2f, 1.8f
		} };

		Mat44f test_m_2 = { {
			2.1f, 0.8f, 4.4f, 8.f,
			7.9f, 3.2f, 3.1f, 7.6f,
			4.9f, 0.8f, 4.f, 7.6f,
			0.6f, 6.f, 6.9f, 6.5f
		} };

		Mat44f expected_result = { {
			68.32f, 73.4f, 87.61f, 124.69f,
			37.8f, 43.68f, 65.43f, 88.22f,
			107.92f, 40.8f, 64.7f, 133.32f,
			81.14f, 34.4f, 79.89f, 144.78f
		} };

		Mat44f result = test_m_1 * test_m_2;

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected_result[i, j], kEps_));
			}
		}

	}

	SECTION("contains 0s")
	{
		Mat44f test_m_1 = { {
			1.0f, 2.0f, 3.0f, 0.0f,
			4.0f, 5.0f, 6.0f, 0.0f,
			7.0f, 8.0f, 9.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		} };

		Mat44f test_m_2 = { {
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 2.0f, 3.0f,
			0.0f, 4.0f, 5.0f, 6.0f,
			0.0f, 7.0f, 8.0f, 9.0f
		} };

		Mat44f expected_result = { {
			0.f, 14.f, 19.f, 24.f,
			0.f, 29.f, 40.f, 51.f,
			0.f, 44.f, 61.f, 78.f,
			0.f,  0.f,  0.f,  0.f,
		} };

		Mat44f result = test_m_1 * test_m_2;

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				REQUIRE_THAT((result[i, j]), WithinAbs(expected_result[i, j], kEps_));
			}
		}

	}

}

TEST_CASE("4x4 matrix-vector multiplication", "[mat44]")
{
	static constexpr float kEps_ = 1e-4f;

	using namespace Catch::Matchers;

	SECTION("Random")
	{
		Mat44f test_m_1 = { {
			0.5f, 2.9f, 7.2f, 0.2f,
			2.1f, 0.5f, 3.0f, 6.6f,
			3.1f, 5.8f, 0.7f, 8.7f,
			1.3f, 1.8f, 5.0f, 8.6f
		} };


		Vec4f test_m_2 = { 7.6f, 9.7f, 7.6f, 3.8f};

		Vec4f expected_result = {87.41f, 68.69f, 118.2f, 98.02f};

		Vec4f result = test_m_1 * test_m_2;

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			REQUIRE_THAT((result[i]), WithinAbs(expected_result[i], kEps_));
		}

	}

	SECTION("All 0's")
	{
		Mat44f test_m_1 = { {
			0.5f, 2.9f, 7.2f, 0.2f,
			2.1f, 0.5f, 3.0f, 6.6f,
			3.1f, 5.8f, 0.7f, 8.7f,
			1.3f, 1.8f, 5.0f, 8.6f
		} };


		Vec4f test_m_2 = { 0.f, 0.f, 0.f, 0.f };

		Vec4f expected_result = { 0.f, 0.f, 0.f, 0.f };

		Vec4f result = test_m_1 * test_m_2;

		//check result matches expected result
		for (int i = 0; i < 4; i++) {
			REQUIRE_THAT((result[i]), WithinAbs(expected_result[i], kEps_));
		}

	}

}

