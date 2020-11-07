#pragma once

#include <string>
#include "SDL.h"

#include "foundation/graphics/color.h"
#include "foundation/graphics/imanaged.h"
#include "foundation/base/types.h"
#include "utils/bit_array.h"

namespace Foundation
{
	class Viewport;
	class WrappedString;

	class Cursor final
	{
		friend class TextField;

	public:
		Cursor();

		bool operator==(const Cursor& inOther) const;
		bool operator!=(const Cursor& inOther) const;

		bool IsEnabled() const;
		void SetEnabled(bool inEnabled);

		void SetPosition(int inX, int inY);
		void SetDimensions(int inWidth, int inHeight);

		bool IsPositionInside(int inX, int inY) const;

	private:
		bool m_Enabled;

		int m_X;
		int m_Y;
		int m_Width;
		int m_Height;
	};

	class TextField final : public IManaged
	{
		friend class Viewport;

	private:
		TextField(const Viewport& inViewport, SDL_Renderer* inRenderer, int inWidth, int inHeight, int inX, int inY);
		~TextField();

	public:
		enum class HorizontalAlignment : int
		{
			Left,
			Center,
			Right
		};

		enum class VerticalAlignment : int
		{
			Top,
			Center,
			Bottom
		};

		void SetEnable(bool inEnable);
		bool IsEnabled() const;

		void SetPosition(const Point& inPosition);
		void SetPositionToCenterOfViewport();
		
		const Point& GetPosition() const;
		const Extent& GetDimensions() const;

		Point GetCellPositionFromPixelPosition(const Point& inPixelPosition) const;

		Cursor& GetCursor();

		void Begin() override;
		void End() override;

		void Clear();
		void Clear(int inX, int inY, int inWidth, int inHeight);
		void Clear(const Rect& inRect);

		void ClearText();
		void ClearText(int inX, int inY, int inWidth, int inHeight);
		void ClearText(const Rect& inRect);

		void ColorAreaBackground(const Color& inColor);
		void ColorAreaBackground(const Color& inColor, int inX, int inY, int inWidth, int inHeight);
		void ColorAreaBackground(const Color& inColor, const Rect& inRect);

		void ColorArea(const TextColoring& inColor);
		void ColorArea(const TextColoring& inColor, int inX, int inY, int inWidth, int inHeight);
		void ColorArea(const TextColoring& inColor, const Rect& inRect);

		void Print(int inX, int inY, const std::string& inString);
		void Print(int inX, int inY, const TextColoring& inPrintContext, const std::string& inString);
		void Print(int inX, int inY, const TextColoring& inPrintContext, const std::string& inString, unsigned int inMaxLength);
		void Print(const Point& inPosition, const std::string& inString);
		void Print(const Point& inPosition, const TextColoring& inPrintContext, const std::string& inString);
		void PrintAligned(const Rect& inRect, const WrappedString& inWrappedText, HorizontalAlignment inHorizontalAlignment);
		void PrintAligned(const Rect& inRect, const TextColoring& inTextColoring, const WrappedString& inWrappedText, HorizontalAlignment inHorizontalAlignment);
		void PrintHexValue(int inX, int inY, bool inUppercase, unsigned char inValue);
		void PrintHexValue(int inX, int inY, const TextColoring& inPrintContext, bool inUppercase, unsigned char inValue);
		void PrintHexValue(int inX, int inY, bool inUppercase, unsigned short inValue);
		void PrintHexValue(int inX, int inY, const TextColoring& inPrintContext, bool inUppercase, unsigned short inValue);
		void PrintChar(int inX, int inY, const char inCharacter);
		void PrintChar(int inX, int inY, const TextColoring& inPrintContext, const char inCharacter);

		void ReflectToRenderSurface();

		static const int font_width = 8;
		static const int font_height = 16;
		static const int font_pitch = 1;

	private:
		bool m_Enabled;

		Point m_Position;
		Extent m_Dimensions;

		int m_ResolutionX;
		int m_ResolutionY;

		const Viewport& m_Viewport;

		SDL_Renderer* m_Renderer;
		SDL_Surface* m_Surface;
		SDL_Texture* m_Texture;

		char* m_ScreenCharacterCellBuffer;
		unsigned short* m_ScreenColorCellBuffer;

		Utility::BitArray m_ScreenDirtyCell;

		Cursor m_Cursor;
		Cursor m_CursorLast;
	};
}