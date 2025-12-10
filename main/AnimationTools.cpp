#include "AnimationTools.hpp"


KeyFramedFloat::KeyFramedFloat()
	: mCurrentKFIndex( 0 )
	, mTimeOnCurrentKF( 0.f )
	, mTotalTimeElapsed( 0.f )
	, mCurrentValue( 0.f )
	, mIsFinished( false )
	, mIsPlaying( false )
{
}


KeyFramedFloat::KeyFramedFloat( FloatKeyFrame initial )
	: mCurrentKFIndex( 0 )
	, mTimeOnCurrentKF( 0.f )
	, mTotalTimeElapsed( 0.f )
	, mCurrentValue( initial.mValue )
	, mIsFinished( false )
	, mIsPlaying( false )
{
	mKeyFrames.emplace_back( std::move(initial) );
}


void KeyFramedFloat::Play()
{
	mIsPlaying = true;
}


void KeyFramedFloat::Pause()
{
	mIsPlaying = false;
}


void KeyFramedFloat::Stop()
{
	mCurrentKFIndex = 0;
	mTimeOnCurrentKF = 0.f;
	mTotalTimeElapsed = 0.f;
	mIsFinished = false;
	mIsPlaying = false;
}


void KeyFramedFloat::Toggle()
{
	mIsPlaying = !mIsPlaying;
}


bool KeyFramedFloat::IsPlaying()
{
	return mIsPlaying;
}


float KeyFramedFloat::Update( float aDeltaTime )
{
	if ( mIsFinished || mKeyFrames.size() < 2 )
	{
		return mKeyFrames.empty() ? 0.f : mKeyFrames.back().mValue;
	}


	if( mIsPlaying )
	{
		mTotalTimeElapsed += aDeltaTime;
		mTimeOnCurrentKF += aDeltaTime;
	}


	const FloatKeyFrame& currentKF = mKeyFrames[ mCurrentKFIndex ];
	const FloatKeyFrame& nextKF    = mKeyFrames[ mCurrentKFIndex + 1 ];

	float interpolationProgress = nextKF.mDuration != 0.f ? std::min(mTimeOnCurrentKF / nextKF.mDuration, 1.f) : 1.f;

	mCurrentValue = Lerp(currentKF.mValue, nextKF.mValue, nextKF.mShapingFunc(interpolationProgress));

	if( interpolationProgress == 1.f )
	{
		mCurrentKFIndex++;
		mTimeOnCurrentKF = 0.f;

		if( mCurrentKFIndex == (mKeyFrames.size() - 1) )
		{
			TriggerCallbacks();
			mIsFinished = true;
			mIsPlaying = false;
		}
	}

	return mCurrentValue;
}


void KeyFramedFloat::InsertKeyframe( FloatKeyFrame aKf )
{
 	mKeyFrames.emplace_back( std::move(aKf) );
}


void KeyFramedFloat::InsertOnFinishCallback( std::function<void()> cb )
{
	mOnFinishCallbacks.push_back( cb );
}


float KeyFramedFloat::GetCurrentValue()
{
	return mCurrentValue;
}


void KeyFramedFloat::TriggerCallbacks()
{
	for( auto& cb : mOnFinishCallbacks )
	{
		cb();
	}
}



FloatKeyFrameGenerator::FloatKeyFrameGenerator( float initialValue )
	: mCurrentValue(initialValue)
	, mInitialValue(initialValue)
{
}


FloatKeyFrameGenerator::FloatKeyFrameGenerator( const FloatKeyFrame& initialKF )
	: mCurrentValue(initialKF.mValue)
	, mInitialValue(initialKF.mValue)
{
}


FloatKeyFrame FloatKeyFrameGenerator::GenerateNext( float increment, float duration, std::function<float(float)> shape )
{
	mCurrentValue += increment;
	return { mCurrentValue, duration, shape };
}


FloatKeyFrame FloatKeyFrameGenerator::GenerateWithValue( float value, float duration, std::function<float(float)> shape )
{
	mCurrentValue = value;
	return { mCurrentValue, duration, shape };
}


void FloatKeyFrameGenerator::ResetValue()
{
	mCurrentValue = mInitialValue;
}


void FloatKeyFrameGenerator::SetValue( float value )
{
	mCurrentValue = value;
}


float FloatKeyFrameGenerator::GetValue()
{
	return mCurrentValue;
}
