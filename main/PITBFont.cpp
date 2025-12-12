#include "PITBFont.hpp"


PITBStyle::PITBStyle(int aFont, float aFontSize, uint32_t aColour, int alignment /* = FONS_ALIGN_LEFT*/)
	: mFontSize( aFontSize )
	, mColour( aColour )
	, mAlignment( alignment )
{
	if( FONS_INVALID == aFont )
	{
		throw std::runtime_error("Invalid font!");
	}
	else
	{
		mFont = aFont;
	}
}


int PITBStyle::GetFont() const
{
	return mFont;
}


void PITBStyle::SetFont( int aFont )
{
	if( FONS_INVALID == aFont )
	{
		throw std::runtime_error("Invalid font!");
	}
	else
	{
		mFont = aFont;
	}
}


float PITBStyle::GetFontSize() const
{
	return mFontSize;
}


void PITBStyle::SetFontSize( float aSize )
{
	mFontSize = aSize;
}


uint32_t PITBStyle::GetColour() const
{
	return mColour;
}


void PITBStyle::SetColour( uint32_t aColour )
{
	mColour = aColour;
}

int PITBStyle::GetAlignment() const
{
	return mAlignment;
}


void PITBStyle::SetAlignment(  int alignment )
{
	mAlignment = alignment;
}





PITBText::PITBText(PITBStyleID aStyle, Vec2f aPositionScreen, std::string aString)
	: mFontStyle( aStyle )
	, mString( std::move(aString) )
	, mScreenLocation( aPositionScreen )
{
}


const std::string& PITBText::GetString() const
{
	return mString;
}


PITBStyleID PITBText::GetStyleID() const
{
	return mFontStyle;
}


void PITBText::SetStyleID(PITBStyleID aID)
{
	mFontStyle = aID;
}


Vec2f PITBText::GetScreenLocation() const
{
	return mScreenLocation;
}


void PITBText::SetScreenLocation( Vec2f aLocation )
{
	mScreenLocation = aLocation;
}





PITBFontManager::PITBFontManager()
	: mShaderProgram(nullptr)
{
	mFs = CreateFons(2048, 2048, FONS_ZERO_TOPLEFT);
}


PITBFontManager::~PITBFontManager()
{
	DeleteFons(mFs);
}


PITBFontManager& PITBFontManager::Get()
{
	static PITBFontManager sInstance;
	return sInstance;
}


void PITBFontManager::SetShaderProgram(ShaderProgram* aShaderProgram)
{
	mShaderProgram = aShaderProgram;
}


void PITBFontManager::Update(float fbWidth, float fbHeight)
{
	if (mShaderProgram == nullptr)
	{
		throw std::runtime_error("PBIT Font Manager shader program is not loaded!");
	}


	glUseProgram(mShaderProgram->programId());
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); // Enter blending mode to use transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	GLint uViewPortDimensions = glGetUniformLocation( mShaderProgram->programId(), "uViewPortDimensions" );
	Vec2f viewPortDims{ fbWidth, fbHeight };
	glUniform2fv(uViewPortDimensions, 1, &viewPortDims.x);

	fonsClearState(mFs);

	for (auto& text : mTexts)
	{

		const PITBStyle& style = mFontStyles.at(text.GetStyleID().mID);

		// Calculate relative fontsize
		float relativeFontSize = fbHeight * style.GetFontSize();

		float relativeScreenX = fbWidth * text.mScreenLocation.x;
		float relativeScreenY = fbHeight * text.mScreenLocation.y;

		fonsSetSize(mFs, relativeFontSize);
		fonsSetFont(mFs, style.GetFont());
		fonsSetColor(mFs, style.GetColour());
		fonsSetAlign(mFs, style.GetAlignment());

		fonsDrawText(mFs, relativeScreenX, relativeScreenY + relativeFontSize, text.mString.c_str(), NULL);
	}

	glUseProgram(0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}


PITBStyleID PITBFontManager::MakeStyle(const char* aFontPath, float aFontSize, uint32_t aColour, int aAlignment /*= FONS_ALIGN_LEFT*/)
{
	int font = fonsAddFont(mFs, aFontPath, aFontPath);
	mFontStyles.emplace_back(font, aFontSize, aColour, aAlignment);

	return PITBStyleID(mFontStyles.size() - 1);
}


PITBText& PITBFontManager::MakeText(PITBStyleID aStyle, Vec2f aPositionRelative, std::string aString)
{
	return mTexts.emplace_back(aStyle, aPositionRelative, std::move(aString));
}


PITBStyleID PITBFontManager::MakeStyleDerived(PITBStyleID styleID, float aFontSize, uint32_t aColour, int aAlignment /*= FONS_ALIGN_LEFT*/)
{
	int font = GetStyle(styleID).GetFont();
	mFontStyles.emplace_back(font, aFontSize, aColour, aAlignment);

	return PITBStyleID(mFontStyles.size() - 1);
}



PITBStyle& PITBFontManager::GetStyle( PITBStyleID aID )
{
	return mFontStyles.at(aID.mID);
}
