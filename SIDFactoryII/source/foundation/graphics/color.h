#pragma once 

#include <limits>

namespace Foundation
{
	enum class Color : unsigned short
	{
		Black,				//  0
		White,				//  1
		Red,				//  2
		Green,				//  3
		Blue,				//  4
		Yellow,				//  5
		LightRed,			//  6
		LightGreen,			//  7
		LightBlue,			//  8
		LightYellow,		//  9
		DarkRed,			// 10
		DarkGreen,			// 11
		DarkBlue,			// 12
		DarkYellow,			// 13
		LighterGrey,		// 14
		LightGrey,			// 15
		Grey,				// 16
		DarkGrey,			// 17
		DarkerGrey,			// 18
		DarkerRed,			// 19
		DarkerGreen,		// 20
		DarkerBlue,			// 21
		DarkerYellow,		// 22

		UserColor00 = 0x40,
	};


	class Palette final
	{
	public:
		Palette();

		void SetUserColor(unsigned char inUserColorIndex, unsigned int inARGB);
		unsigned int GetColorARGB(Color inColor) const;

	private:
		unsigned int m_Colors[0x100];
	};


	class TextColoring
	{
	public:
		TextColoring()
			: m_ForegroundColor(Color::White)
			, m_BackgroundColor(Color::Black)
			, m_ChangeBackgroundColor(false)
		{

		}

		TextColoring(Color inForegroundColor)
			: m_ForegroundColor(inForegroundColor)
			, m_BackgroundColor(Color::Black)
			, m_ChangeBackgroundColor(false)
		{

		}

		TextColoring(Color inForegroundColor, Color inBackgroundColor)
			: m_ForegroundColor(inForegroundColor)
			, m_BackgroundColor(inBackgroundColor)
			, m_ChangeBackgroundColor(true)
		{

		}

		void SetForegroundColor(Color inColor);
		void SetBackgroundColor(Color inColor);
		void SetChangeBackgroundColor(bool inChangeBackgroundColor);

		Color GetForegroundColor() const;
		Color GetBackgroundColor() const;
		bool GetChangeBackgroundColor() const;

		unsigned short GetColorCellValue() const;

	private:
		Color m_ForegroundColor;
		Color m_BackgroundColor;

		bool m_ChangeBackgroundColor;
	};
}