#ifndef PITB_FONT_HPP
#define PITB_FONT_HPP





#include "../vmlib/vec4.hpp"
#include "../vmlib/vec2.hpp"
#include <string>
#include <format>
#include <vector>
#include <deque>
#include <unordered_map>
#include "fontstashbackend.hpp"
#include "../support/program.hpp"




/*
*	Scroll down for manual
*	It should be above the PITBFontManager class
*/




class PITBStyle
{
public:
	friend class PITBFontManager;

	int GetFont() const;
	void SetFont( int aFont );

	float GetFontSize() const;
	void SetFontSize( float aSize );

	uint32_t GetColour() const;
	void SetColour( uint32_t aColour );

	int GetAlignment() const;
	void SetAlignment(  int alignment );


public:
	// Ideally these would be private but since we are storing this in
	// the vector, it can't be private.
	// So please don't construct this yourself.
	PITBStyle(int font, float aFontSizeRelative, uint32_t aColour, int alignment = FONS_ALIGN_LEFT);

	~PITBStyle() = default;

	PITBStyle(const PITBStyle&) = delete;
	PITBStyle& operator=(const PITBStyle&) = delete;

	PITBStyle(PITBStyle&&) = default;
	PITBStyle& operator=(PITBStyle&&) = default;


private:
	int mFont;
	float mFontSize;
	uint32_t mColour;
	int mAlignment;
};





class PITBStyleID
{
public:
	friend class PITBFontManager;

	bool operator==(const PITBStyleID& other) const noexcept
	{
		return mID == other.mID;
	}


private:
	PITBStyleID(size_t aID) : mID(aID) {};
	size_t mID;
};





class PITBText
{
public:
	friend class PITBFontManager;

	const std::string& GetString() const;

	template <typename... Ts>
	void SetString(std::format_string<Ts...> fmt, Ts&&... args)
	{
		mString = std::format(fmt, std::forward<Ts>(args)...);
	}


	PITBStyleID GetStyleID() const;
	void SetStyleID(PITBStyleID aId);

	Vec2f GetScreenLocation() const;
	void SetScreenLocation( Vec2f aLocation );


public:
	// Ideally these would also be private but since we are storing this in
	// the queue, it can't be private.
	// So please don't construct this yourself.
	PITBText(PITBStyleID aStyle, Vec2f aPositionScreen, std::string aString);

	~PITBText() = default;

	PITBText(const PITBText&) = delete;
	PITBText& operator=(const PITBText&) = delete;

	PITBText(PITBText&&) = default;
	PITBText& operator=(PITBText&&) = default;


private:
	PITBStyleID mFontStyle;
	std::string mString;
	Vec2f mScreenLocation;
};





// Singleton object that manages all strings on the screen
/*
* PITB : Pretty Interesting Text Bureau (yeah totally, because this was such an interesting
*		 thing and totally 100% absolutely not a pain...).
*
* Usage: call Get() to initialize the singleton sometime before your render loop,
*		 then attach the shader program by calling SetShaderProgram()
*
*		 Make a style first. call MakeStyle, give it your font file, font size (relative to screen
*		 height between 0 and 1), and colour.
*		 Save the returned PITBStyleID we'll need it to make text.
*
*		 Now to make text call MakeText(), give it your PTIBStyleID to set your text style,
*		 give it the screen position relative (between 0 and 1), and then give it your actual string
*		 (supports fmt strings btw isn't that cool?) Also save the returned PTIBText reference. You
*		 might need it to modify the text.
*
*		 Also don't forget to call Update() every frame!!!
*/
class PITBFontManager
{
public:
	static PITBFontManager& Get();

	void SetShaderProgram(ShaderProgram* mShaderProgram);

	void Update(float fbWidth, float fbHeight);


	template <typename... Ts>
	PITBText& MakeText(PITBStyleID aStyle, Vec2f aPositionRelative, std::format_string<Ts...> fmt, Ts&&... args)
	{
		std::string tmp = std::format(fmt, std::forward<Ts>(args)...);
		return mTexts.emplace_back(aStyle, aPositionRelative, tmp);
	};


	PITBText& MakeText(PITBStyleID aStyle, Vec2f aPositionRelative, std::string aString);


	PITBStyleID MakeStyle(const char* aFontPath, float aFontSize, uint32_t aColour, int aAlignment = FONS_ALIGN_LEFT);


	PITBStyleID MakeStyleDerived(PITBStyleID styleID, float aFontSize, uint32_t aColour, int aAlignment = FONS_ALIGN_LEFT);


	PITBStyle& GetStyle( PITBStyleID aID );


private:
	PITBFontManager();
	~PITBFontManager();

	PITBFontManager(PITBFontManager&) = delete;
	PITBFontManager operator=(PITBFontManager&) = delete;


private:
	FONScontext* mFs;
	ShaderProgram* mShaderProgram;
	std::deque<PITBText> mTexts;
	std::vector<PITBStyle> mFontStyles;
};





#endif // PITB_FONT_HPP




