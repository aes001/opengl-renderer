#ifndef ANIMATION_TOOLS_HPP_BB4E6247_00A2_44E4_8AD3_D48FA149D66E
#define ANIMATION_TOOLS_HPP_BB4E6247_00A2_44E4_8AD3_D48FA149D66E


#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include <functional>
#include <algorithm>


constexpr
float Lerp(float a, float b, float t) noexcept
{
	return a * ( 1.f - t ) + b * t;
}


// Premade Shaping functions
// We can also make our own using lambdas
// Input of t should be between 0 and 1
namespace ShapingFunctions
{
	constexpr
	float None( float t )
	{
		(void) t;
		return 0.f;
	}


	constexpr
	float Instant( float t )
	{
		(void) t;
		return 1.f;
	}


	constexpr
	float Linear( float t )
	{
		return t;
	}


	template <int DEGREE>
	constexpr float Polynomial( float t )
	{
		if constexpr (DEGREE == 1)
			return t;
		else
			return t * Polynomial<DEGREE-1>(t);
	}


	template <int DEGREE>
	constexpr float PolynomialEaseOut( float t )
	{
		return 1 - Polynomial<DEGREE>(1 - t);
	}


	constexpr float Smoothstep( float t )
	{
		float a = Polynomial<2>(t); // x^2
		float b = 1.f - Polynomial<2>(1.f - t); // 1-(1-x)^2
		return Lerp(a, b, t);
	}
}




struct FloatKeyFrame
{
	float mValue;
	float mDuration;
	std::function<float(float)> mShapingFunc;
};





// ===========================================================================
//		KeyFramedFloat
// ---------------------------------------------------------------------------
//		Description
// ---------------------------------------------------------------------------
//	Represents an animated float. Call .Update(dt) will return the value of
//	the float after the set amount of time has elapsed.
//	Controls are a bit like media player controls, play, pause, stop...
// ---------------------------------------------------------------------------
class KeyFramedFloat
{
public:
	KeyFramedFloat();
	KeyFramedFloat( FloatKeyFrame inital );

	void Play();
	void Pause();
	void Stop();
	void Toggle();

	bool IsPlaying();

	float Update(float deltaTime);

	void InsertKeyframe( FloatKeyFrame kf );

	void InsertOnFinishCallback( std::function<void()> cb );

	float GetCurrentValue();


private:
	void TriggerCallbacks();


private:
	std::vector<FloatKeyFrame> mKeyFrames;
	std::vector<std::function<void()>> mOnFinishCallbacks;
	size_t mCurrentKFIndex;
	float mTimeOnCurrentKF;
	float mTotalTimeElapsed;
	float mCurrentValue;
	bool mIsFinished;
	bool mIsPlaying;
};





// ===========================================================================
//		FloatKeyFrameGenerator
// ---------------------------------------------------------------------------
//		Description
// ---------------------------------------------------------------------------
//	Helper object to generate key frames incrementally by keeping track of
//	the incremented value
// ---------------------------------------------------------------------------
class FloatKeyFrameGenerator
{
public:
	FloatKeyFrameGenerator( float initalValue );
	FloatKeyFrameGenerator( const FloatKeyFrame& initalKF );

	FloatKeyFrame GenerateNext( float increment, float duration, std::function<float(float)> shape );
	FloatKeyFrame GenerateWithValue( float value, float duration, std::function<float(float)> shape );

	void ResetValue();
	void SetValue( float value );

	float GetValue();

private:
	float mCurrentValue;
	float mInitialValue;
};





#endif // ANIMATION_TOOLS_HPP_BB4E6247_00A2_44E4_8AD3_D48FA149D66E
