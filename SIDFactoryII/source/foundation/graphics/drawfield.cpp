#include "drawfield.h"
#include "viewport.h"

#include "foundation/base/assert.h"

namespace Foundation
{
	DrawField::DrawField(const Viewport& inViewport, SDL_Renderer* inRenderer, int inWidth, int inHeight, int inX, int inY)
		: m_Viewport(inViewport)
		, m_Renderer(inRenderer)
		, m_Position({ inX, inY })
		, m_Dimensions({ inWidth, inHeight })
		, m_Enabled(false)
	{
		m_Surface = SDL_CreateRGBSurface(0, inWidth, inHeight, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		FOUNDATION_ASSERT(m_Surface);

		m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, inWidth, inHeight);
		FOUNDATION_ASSERT(m_Texture);
	}


	DrawField::~DrawField()
	{
		SDL_FreeSurface(m_Surface);
		SDL_DestroyTexture(m_Texture);
	}


	void DrawField::SetEnable(bool inEnable)
	{
		m_Enabled = inEnable;
	}


	bool DrawField::IsEnabled() const
	{
		return m_Enabled;
	}


	void DrawField::Begin()
	{
		SDL_LockSurface(m_Surface);
	}

	void DrawField::End()
	{
		SDL_UnlockSurface(m_Surface);
		SDL_UpdateTexture(m_Texture, nullptr, m_Surface->pixels, m_Surface->pitch);

		if (m_Enabled)
		{
			SDL_Rect rect;

			rect.x = m_Position.m_X;
			rect.y = m_Position.m_Y;
			rect.w = m_Dimensions.m_Width;
			rect.h = m_Dimensions.m_Height;

			SDL_RenderCopy(m_Renderer, m_Texture, nullptr, &rect);
		}
	}


	const Extent& DrawField::GetDimensions() const
	{
		return m_Dimensions;
	}


	const Point& DrawField::GetPosition() const
	{
		return m_Position;
	}


	void DrawField::SetPosition(const Point& inPosition)
	{
		m_Position = inPosition;
	}


	void DrawField::Clear(const Color& inColor)
	{
		FOUNDATION_ASSERT(m_Surface != nullptr);

		const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);
		unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);

		for (int y = 0; y < m_Dimensions.m_Height; ++y)
		{
			unsigned int* dest_line = reinterpret_cast<unsigned int*>(buffer + m_Surface->pitch * y);
		
			for (int x = 0; x < m_Dimensions.m_Width; ++x)
				dest_line[x] = color;
		}
	}


	void DrawField::DrawDot(const Color& inColor, int inX, int inY)
	{
		FOUNDATION_ASSERT(m_Surface != nullptr);

		if (m_Dimensions.Contains({ inX, inY }))
		{
			const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);
			unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);

			unsigned int* dest_line = reinterpret_cast<unsigned int*>(buffer + m_Surface->pitch * inY);
			dest_line[inX] = color;
		}
	}


	void DrawField::DrawLine(const Color& inColor, int inX1, int inY1, int inX2, int inY2)
	{
		FOUNDATION_ASSERT(m_Surface != nullptr);

		const int lenX = abs(inX1 - inX2);
		const int lenY = abs(inY1 - inY2);

		if (lenX > lenY)
		{
			if (inX1 > inX2)
			{
				int t = inX1;
				inX1 = inX2;
				inX2 = t;
				t = inY1;
				inY1 = inY2;
				inY2 = t;
			}

			if (lenX > 0 && inX1 < m_Dimensions.m_Width && inX2 >= 0)
			{
				const float factor = static_cast<float>(inY2 - inY1) / (inX2 - inX1);
				const int xF = inX1 < 0 ? 0 : inX1;
				const int xT = inX2 > m_Dimensions.m_Width ? m_Dimensions.m_Width : inX2;

				const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);
				unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);

				for (int x = xF; x < xT; ++x)
				{
					int y = static_cast<int>((x - inX1) * factor) + inY1;
					if (y >= 0 && y < m_Dimensions.m_Height)
					{
						unsigned int* dest_line = reinterpret_cast<unsigned int*>(buffer + m_Surface->pitch * y);
						dest_line[x] = color;
					}
				}
			}
		}
		else
		{
			if (inY1 > inY2)
			{
				int t = inY1;
				inY1 = inY2;
				inY2 = t;
				t = inX1;
				inX1 = inX2;
				inX2 = t;
			}

			if (lenY > 0 && inY1 < m_Dimensions.m_Height && inY2 >= 0)
			{
				const float factor = static_cast<float>(inX2 - inX1) / (inY2 - inY1);
				const int yF = inY1 < 0 ? 0 : inY1;
				const int yT = inY2 > m_Dimensions.m_Height ? m_Dimensions.m_Height : inY2;

				const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);
				unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);

				for (int y = yF; y < yT; ++y)
				{
					int x = static_cast<int>((y - inY1) * factor) + inX1;
					if (x >= 0 && x < m_Dimensions.m_Width)
					{
						unsigned int* dest_line = reinterpret_cast<unsigned int*>(buffer + m_Surface->pitch * y);
						dest_line[x] = color;
					}
				}
			}
		}
	}


	void DrawField::DrawBox(const Color& inColor, int inTopLeftX1, int inTopLeftY1, int inWidth, int inHeight)
	{
		unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);
		const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);

		if (inTopLeftX1 < 0)
		{
			inWidth += inTopLeftX1;
			inTopLeftX1 = 0;
		}

		if (inTopLeftY1 < 0)
		{
			inHeight += inTopLeftY1;
			inTopLeftY1 = 0;
		}

		if (inWidth + inTopLeftX1 > m_Dimensions.m_Width)
			inWidth -= inWidth + inTopLeftX1 - m_Dimensions.m_Width;
		if (inHeight + inTopLeftY1 > m_Dimensions.m_Height)
			inHeight -= inHeight + inTopLeftY1 - m_Dimensions.m_Height;

		int y = inTopLeftY1 * m_Surface->pitch;

		for (int i = 0; i < inHeight; ++i)
		{
			unsigned int* dest_draw = reinterpret_cast<unsigned int*>(buffer + y);

			for (int j = inTopLeftX1; j < inTopLeftX1 + inWidth; ++j)
				dest_draw[j] = color;

			y += m_Surface->pitch;
		}
	}


	void DrawField::DrawVerticalLine(const Color& inColor, int inX, int inY1, int inY2)
	{
		unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);
		const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);

		int top = inY1 < inY2 ? inY1 : inY2;
		int bottom = inY1 < inY2 ? inY2 : inY1;

		if (top >= m_Dimensions.m_Height || bottom < 0)
			return;
		if (inX < 0 || inX >= m_Dimensions.m_Width)
			return;

		if (top < 0)
			top = 0;
		if (bottom >= m_Dimensions.m_Height)
			bottom = m_Dimensions.m_Height - 1;
		
		int y = top * m_Surface->pitch;

		for (int i = top; i <= bottom; ++i)
		{
			unsigned int* dest_draw = reinterpret_cast<unsigned int*>(buffer + y);
			dest_draw[inX] = color;

			y += m_Surface->pitch;
		}
	}


	void DrawField::DrawHorizontalLine(const Color& inColor, int inX1, int inX2, int inY)
	{
		unsigned char* buffer = static_cast<unsigned char*>(m_Surface->pixels);
		const unsigned int color = m_Viewport.GetPalette().GetColorARGB(inColor);

		int left = inX1 < inX2 ? inX1 : inX2;
		int right = inX1 < inX2 ? inX2 : inX1;

		if (left >= m_Dimensions.m_Width || right < 0)
			return;
		if (inY < 0 || inY >= m_Dimensions.m_Height)
			return;

		if (left < 0)
			left = 0;
		if (right >= m_Dimensions.m_Width)
			right = m_Dimensions.m_Width - 1;

		int y = inY * m_Surface->pitch;
		unsigned int* dest_draw = reinterpret_cast<unsigned int*>(buffer + y);

		for (int i = left; i <= right; ++i)
			dest_draw[i] = color;
	}
}
