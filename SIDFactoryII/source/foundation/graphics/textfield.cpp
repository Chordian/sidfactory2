#include "textfield.h"
#include "resources/data_char.h"
#include "viewport.h"
#include "wrapped_string.h"

#include "SDL.h"
#include "foundation/base/assert.h"

namespace Foundation
{
	void TextColoring::SetForegroundColor(Color inColor)
	{
		m_ForegroundColor = Color(static_cast<unsigned short>(inColor) & 0xff);
	}

	void TextColoring::SetBackgroundColor(Color inColor)
	{
		m_BackgroundColor = Color(static_cast<unsigned short>(inColor) & 0xff);
	}

	void TextColoring::SetChangeBackgroundColor(bool inChangeBackgroundColor)
	{
		m_ChangeBackgroundColor = inChangeBackgroundColor;
	}

	Color TextColoring::GetForegroundColor() const
	{
		return m_ForegroundColor;
	}

	Color TextColoring::GetBackgroundColor() const
	{
		return m_BackgroundColor;
	}

	bool TextColoring::GetChangeBackgroundColor() const
	{
		return m_ChangeBackgroundColor;
	}

	unsigned short TextColoring::GetColorCellValue() const
	{
		if (!m_ChangeBackgroundColor)
			return static_cast<unsigned short>(m_ForegroundColor);

		return static_cast<unsigned short>(m_ForegroundColor) | (static_cast<unsigned short>(m_BackgroundColor) << 8);
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	Cursor::Cursor()
		: m_Enabled(false)
		, m_X(0)
		, m_Y(0)
		, m_Width(1)
		, m_Height(1)
	{
	}

	bool Cursor::operator==(const Cursor& inOther) const
	{
		return (m_Enabled == inOther.m_Enabled
			&& m_X == inOther.m_X
			&& m_Y == inOther.m_Y
			&& m_Width == inOther.m_Width
			&& m_Height == inOther.m_Height);
	}

	bool Cursor::operator!=(const Cursor& inOther) const
	{
		return !(*this == inOther);
	}


	bool Cursor::IsEnabled() const
	{
		return m_Enabled;
	}

	void Cursor::SetEnabled(bool inEnabled)
	{
		m_Enabled = inEnabled;
	}

	void Cursor::SetPosition(int inX, int inY)
	{
		m_X = inX;
		m_Y = inY;
	}

	void Cursor::SetDimensions(int inWidth, int inHeight)
	{
		m_Width = inWidth;
		m_Height = inHeight;
	}

	bool Cursor::IsPositionInside(int inX, int inY) const
	{
		return (inX >= m_X && inY >= m_Y && inX < m_X + m_Width && inY < m_Y + m_Height);
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	TextField::TextField(const Viewport& inViewport, SDL_Renderer* inRenderer, int inWidth, int inHeight, int inX, int inY)
		: m_Viewport(inViewport)
		, m_Renderer(inRenderer)
		, m_Position({ inX, inY })
		, m_Dimensions({ inWidth, inHeight })
		, m_ResolutionX(inWidth * inViewport.GetFont().width)
		, m_ResolutionY(inHeight * inViewport.GetFont().height)
		, m_Enabled(false)
	{
		m_Surface = SDL_CreateRGBSurface(0, m_ResolutionX, m_ResolutionY, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		FOUNDATION_ASSERT(m_Surface);

		m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_ResolutionX, m_ResolutionY);
		FOUNDATION_ASSERT(m_Texture);

		const int cell_buffer_size = m_Dimensions.m_Width * m_Dimensions.m_Height;

		m_ScreenCharacterCellBuffer = new char[cell_buffer_size];
		m_ScreenColorCellBuffer = new unsigned short[cell_buffer_size];
		m_ScreenDirtyCell.Resize(cell_buffer_size);

		memset(m_ScreenCharacterCellBuffer, 0, cell_buffer_size);
		memset(m_ScreenColorCellBuffer, 0, cell_buffer_size * sizeof(unsigned short));
	}


	TextField::~TextField()
	{
		SDL_FreeSurface(m_Surface);
		SDL_DestroyTexture(m_Texture);
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	void TextField::SetEnable(bool inEnabled)
	{
		m_Enabled = inEnabled;
	}


	bool TextField::IsEnabled() const
	{
		return m_Enabled;
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	void TextField::SetPositionToCenterOfViewport()
	{
		int x = (m_Viewport.GetClientWidth() - m_ResolutionX) >> 1;
		int y = (m_Viewport.GetClientHeight() - m_ResolutionY) >> 1;

		m_Position = { x, y };
	}


	void TextField::SetPosition(const Point& inPosition)
	{
		m_Position = inPosition;
	}


	const Point& TextField::GetPosition() const
	{
		return m_Position;
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	const Extent& TextField::GetDimensions() const
	{
		return m_Dimensions;
	}

	const Resource::Font& TextField::GetFont() const
	{
		return m_Viewport.GetFont();
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	Point TextField::GetCellPositionFromPixelPosition(const Point& inPixelPosition) const
	{
		Foundation::Point local_position = inPixelPosition - m_Position;

		const int cell_x = local_position.m_X / m_Viewport.GetFont().width;
		const int cell_y = local_position.m_Y / m_Viewport.GetFont().height;

		return Foundation::Point({ cell_x, cell_y });
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	Cursor& TextField::GetCursor()
	{
		return m_Cursor;
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	void TextField::Begin()
	{
		SDL_LockSurface(m_Surface);
	}


	void TextField::End()
	{
		ReflectToRenderSurface();

		SDL_UnlockSurface(m_Surface);
		SDL_UpdateTexture(m_Texture, nullptr, m_Surface->pixels, m_Surface->pitch);

		if (m_Enabled)
		{
			SDL_Rect rect;

			rect.x = m_Position.m_X;
			rect.y = m_Position.m_Y;
			rect.w = m_ResolutionX;
			rect.h = m_ResolutionY;

			SDL_RenderCopy(m_Renderer, m_Texture, nullptr, &rect);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	void TextField::Clear()
	{
		const int cell_buffer_size = m_Dimensions.m_Width * m_Dimensions.m_Height;

		for (int i = 0; i < cell_buffer_size; ++i)
		{
			m_ScreenCharacterCellBuffer[i] = 0;
			m_ScreenColorCellBuffer[i] = 0;
			m_ScreenDirtyCell.Set(i);
		}
	}


	void TextField::Clear(int inX, int inY, int inWidth, int inHeight)
	{
		const int x1 = inX < 0 ? 0 : inX;
		const int x2 = inX + inWidth > m_Dimensions.m_Width ? m_Dimensions.m_Width : inX + inWidth;
		const int y1 = inY < 0 ? 0 : inY;
		const int y2 = inY + inHeight > m_Dimensions.m_Height ? m_Dimensions.m_Height : inY + inHeight;

		for (int y = y1; y < y2; ++y)
		{
			int i = y * m_Dimensions.m_Width + x1;

			for (int x = x1; x < x2; ++x, ++i)
			{
				m_ScreenCharacterCellBuffer[i] = 0;
				m_ScreenColorCellBuffer[i] = 0;
				m_ScreenDirtyCell.Set(i);
			}
		}
	}


	void TextField::Clear(const Rect& inRect)
	{
		Clear(inRect.m_Position.m_X, inRect.m_Position.m_Y, inRect.m_Dimensions.m_Width, inRect.m_Dimensions.m_Height);
	}


	void TextField::ClearText()
	{
		const int cell_buffer_size = m_Dimensions.m_Width * m_Dimensions.m_Height;

		for (int i = 0; i < cell_buffer_size; ++i)
		{
			m_ScreenCharacterCellBuffer[i] = 0;
			m_ScreenDirtyCell.Set(i);
		}
	}


	void TextField::ClearText(int inX, int inY, int inWidth, int inHeight)
	{
		const int x1 = inX < 0 ? 0 : inX;
		const int x2 = inX + inWidth > m_Dimensions.m_Width ? m_Dimensions.m_Width : inX + inWidth;
		const int y1 = inY < 0 ? 0 : inY;
		const int y2 = inY + inHeight > m_Dimensions.m_Height ? m_Dimensions.m_Height : inY + inHeight;

		for (int y = y1; y < y2; ++y)
		{
			int i = y * m_Dimensions.m_Width + x1;

			for (int x = x1; x < x2; ++x, ++i)
			{
				if (m_ScreenCharacterCellBuffer[i] != 0)
				{
					m_ScreenCharacterCellBuffer[i] = 0;
					m_ScreenDirtyCell.Set(i);
				}
			}
		}
	}


	void TextField::ClearText(const Rect& inRect)
	{
		ClearText(inRect.m_Position.m_X, inRect.m_Position.m_Y, inRect.m_Dimensions.m_Width, inRect.m_Dimensions.m_Height);
	}


	void TextField::ColorAreaBackground(const Color& inColor)
	{
		ColorAreaBackground(inColor, { { 0, 0 }, m_Dimensions });
	}


	void TextField::ColorAreaBackground(const Color& inColor, int inX, int inY, int inWidth, int inHeight)
	{
		const unsigned short cell_color = (static_cast<unsigned short>(inColor) & 0xff) << 8;

		const int x1 = inX < 0 ? 0 : inX;
		const int x2 = inX + inWidth > m_Dimensions.m_Width ? m_Dimensions.m_Width : inX + inWidth;
		const int y1 = inY < 0 ? 0 : inY;
		const int y2 = inY + inHeight > m_Dimensions.m_Height ? m_Dimensions.m_Height : inY + inHeight;

		for (int y = y1; y < y2; ++y)
		{
			int i = y * m_Dimensions.m_Width + x1;

			for (int x = x1; x < x2; ++x, ++i)
			{
				const unsigned short previous_color_val = m_ScreenColorCellBuffer[i];
				const unsigned short color_val = (previous_color_val & 0x00ff) | cell_color;

				if (color_val != previous_color_val)
				{
					m_ScreenColorCellBuffer[i] = color_val;
					m_ScreenDirtyCell.Set(i);
				}
			}
		}
	}


	void TextField::ColorAreaBackground(const Color& inColor, const Rect& inRect)
	{
		ColorAreaBackground(inColor, inRect.m_Position.m_X, inRect.m_Position.m_Y, inRect.m_Dimensions.m_Width, inRect.m_Dimensions.m_Height);
	}


	void TextField::ColorArea(const TextColoring& inColor)
	{
		ColorArea(inColor, { { 0, 0 }, m_Dimensions });
	}


	void TextField::ColorArea(const TextColoring& inColor, int inX, int inY, int inWidth, int inHeight)
	{
		const unsigned short cell_color = inColor.GetColorCellValue();

		const int x1 = inX < 0 ? 0 : inX;
		const int x2 = inX + inWidth > m_Dimensions.m_Width ? m_Dimensions.m_Width : inX + inWidth;
		const int y1 = inY < 0 ? 0 : inY;
		const int y2 = inY + inHeight > m_Dimensions.m_Height ? m_Dimensions.m_Height : inY + inHeight;

		for (int y = y1; y < y2; ++y)
		{
			int i = y * m_Dimensions.m_Width + x1;

			for (int x = x1; x < x2; ++x, ++i)
			{
				const unsigned short previous_color_val = m_ScreenColorCellBuffer[i];

				if (previous_color_val != cell_color)
				{
					if (inColor.GetChangeBackgroundColor())
					{
						m_ScreenColorCellBuffer[i] = cell_color;
						m_ScreenDirtyCell.Set(i);
					}
					else
					{
						if ((previous_color_val & 0x00ff) != cell_color)
						{
							m_ScreenColorCellBuffer[i] = cell_color | (previous_color_val & 0xff00);
							m_ScreenDirtyCell.Set(i);
						}
					}
				}
			}
		}
	}


	void TextField::ColorArea(const TextColoring& inColor, const Rect& inRect)
	{
		ColorArea(inColor, inRect.m_Position.m_X, inRect.m_Position.m_Y, inRect.m_Dimensions.m_Width, inRect.m_Dimensions.m_Height);
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	void TextField::Print(const Point& inPosition, const std::string& inString)
	{
		Print(inPosition.m_X, inPosition.m_Y, inString);
	}


	void TextField::Print(const Point& inPosition, const TextColoring& inPrintContext, const std::string& inString)
	{
		Print(inPosition.m_X, inPosition.m_Y, inPrintContext, inString);
	}


	void TextField::Print(int inX, int inY, const std::string& inString)
	{
		const TextColoring print_context;
		Print(inX, inY, print_context, inString);
	}


	void TextField::Print(int inX, int inY, const TextColoring& inPrintContext, const std::string& inString)
	{
		const unsigned int length = static_cast<unsigned int>(inString.size());
		Print(inX, inY, inPrintContext, inString, length);
	}


	void TextField::Print(int inX, int inY, const TextColoring& inPrintContext, const std::string& inString, unsigned int inMaxLength)
	{
		if (inX >= 0 && inX < m_Dimensions.m_Width && inY >= 0 && inY < m_Dimensions.m_Height)
		{
			const char* c_string = inString.c_str();
			const unsigned short current_color_cell_value = inPrintContext.GetColorCellValue();

			int base_offset = inY * m_Dimensions.m_Width;
			int x = inX;

			const unsigned int length = static_cast<unsigned int>(inString.length()) < inMaxLength ? static_cast<unsigned int>(inString.length()) : inMaxLength;

			for (unsigned int i = 0; i < length && x < m_Dimensions.m_Width; ++i, ++x)
			{
				bool cell_changed = false;
				int offset = base_offset + x;

				cell_changed |= m_ScreenCharacterCellBuffer[offset] != c_string[i];
				m_ScreenCharacterCellBuffer[offset] = c_string[i];

				if (inPrintContext.GetChangeBackgroundColor())
				{
					cell_changed |= m_ScreenColorCellBuffer[offset] != current_color_cell_value;
					m_ScreenColorCellBuffer[offset] = current_color_cell_value;
				}
				else
				{
					const unsigned short value = (m_ScreenColorCellBuffer[offset] & 0xff00) | current_color_cell_value;

					cell_changed |= m_ScreenColorCellBuffer[offset] != value;
					m_ScreenColorCellBuffer[offset] = value;
				}

				if (cell_changed)
					m_ScreenDirtyCell.Set(offset);
			}
		}
	}


	void TextField::PrintAligned(const Rect& inRect, const WrappedString& inWrappedString, HorizontalAlignment inHorizontalAlignment)
	{
		const TextColoring text_coloring;
		PrintAligned(inRect, text_coloring, inWrappedString, inHorizontalAlignment);
	}


	void TextField::PrintAligned(const Rect& inRect, const TextColoring& inTextColoring, const WrappedString& inWrappedString, HorizontalAlignment inHorizontalAlignment)
	{
		const int width = inRect.m_Dimensions.m_Width;
		int y_offset = 0;

		for (const WrappedString::Line& line : inWrappedString.GetLines())
		{
			int x_offset = [&]()
			{
				if (inHorizontalAlignment == HorizontalAlignment::Center)
					return (width - line.m_Width) >> 1;
				if (inHorizontalAlignment == HorizontalAlignment::Right)
					return (width - line.m_Width);

				return 0;
			}();

			Print(inRect.m_Position.m_X + x_offset, inRect.m_Position.m_Y + y_offset, inTextColoring, line.m_Line);

			++y_offset;

			if (y_offset >= inRect.m_Dimensions.m_Height)
				break;
		}
	}


	void TextField::PrintHexValue(int inX, int inY, bool inUppercase, unsigned char inValue)
	{
		PrintHexValue(inX, inY, TextColoring(), inUppercase, inValue);
	}


	void TextField::PrintHexValue(int inX, int inY, const TextColoring& inPrintContext, bool inUppercase, unsigned char inValue)
	{
		auto make_character = [&inUppercase](unsigned char inValue) -> char
		{
			if (inValue > 0x0f)
				return 'x';
			if (inValue < 10)
				return '0' + inValue;

			if (inUppercase)
				return 'A' + inValue - 10;

			return 'a' + inValue - 10;
		};

		const unsigned short current_color_cell_value = inPrintContext.GetColorCellValue();
		const int offset = inY * m_Dimensions.m_Width + inX;
		const char character_1 = make_character(inValue >> 4);
		const char character_2 = make_character(inValue & 0x0f);

		bool cell_changed = false;

		cell_changed |= m_ScreenCharacterCellBuffer[offset + 0] != character_1;
		cell_changed |= m_ScreenCharacterCellBuffer[offset + 1] != character_2;

		m_ScreenCharacterCellBuffer[offset + 0] = character_1;
		m_ScreenCharacterCellBuffer[offset + 1] = character_2;

		if (inPrintContext.GetChangeBackgroundColor())
		{
			cell_changed |= m_ScreenColorCellBuffer[offset + 0] != current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 1] != current_color_cell_value;

			m_ScreenColorCellBuffer[offset + 0] = current_color_cell_value;
			m_ScreenColorCellBuffer[offset + 1] = current_color_cell_value;
		}
		else
		{
			unsigned short value = (m_ScreenColorCellBuffer[offset + 0] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 0] != value;
			m_ScreenColorCellBuffer[offset + 0] = value;

			value = (m_ScreenColorCellBuffer[offset + 1] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 1] != value;
			m_ScreenColorCellBuffer[offset + 1] = value;
		}

		if (cell_changed)
		{
			m_ScreenDirtyCell.Set(offset + 0);
			m_ScreenDirtyCell.Set(offset + 1);
		}
	}


	void TextField::PrintHexValue(int inX, int inY, bool inUppercase, unsigned short inValue)
	{
		PrintHexValue(inX, inY, TextColoring(), inUppercase, inValue);
	}


	void TextField::PrintHexValue(int inX, int inY, const TextColoring& inPrintContext, bool inUppercase, unsigned short inValue)
	{
		auto make_character = [&inUppercase](unsigned char inValue) -> char
		{
			if (inValue > 0x0f)
				return 'x';
			if (inValue < 10)
				return '0' + inValue;

			if (inUppercase)
				return 'A' + inValue - 10;

			return 'a' + inValue - 10;
		};

		const unsigned short current_color_cell_value = inPrintContext.GetColorCellValue();
		const int offset = inY * m_Dimensions.m_Width + inX;
		const char character_1 = make_character(inValue >> 12);
		const char character_2 = make_character((inValue >> 8) & 0x0f);
		const char character_3 = make_character((inValue >> 4) & 0x0f);
		const char character_4 = make_character(inValue & 0x0f);

		bool cell_changed = false;

		cell_changed |= m_ScreenCharacterCellBuffer[offset + 0] != character_1;
		cell_changed |= m_ScreenCharacterCellBuffer[offset + 1] != character_2;
		cell_changed |= m_ScreenCharacterCellBuffer[offset + 2] != character_3;
		cell_changed |= m_ScreenCharacterCellBuffer[offset + 3] != character_4;

		m_ScreenCharacterCellBuffer[offset + 0] = character_1;
		m_ScreenCharacterCellBuffer[offset + 1] = character_2;
		m_ScreenCharacterCellBuffer[offset + 2] = character_3;
		m_ScreenCharacterCellBuffer[offset + 3] = character_4;

		if (inPrintContext.GetChangeBackgroundColor())
		{
			cell_changed |= m_ScreenColorCellBuffer[offset + 0] != current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 1] != current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 2] != current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 3] != current_color_cell_value;

			m_ScreenColorCellBuffer[offset + 0] = current_color_cell_value;
			m_ScreenColorCellBuffer[offset + 1] = current_color_cell_value;
			m_ScreenColorCellBuffer[offset + 2] = current_color_cell_value;
			m_ScreenColorCellBuffer[offset + 3] = current_color_cell_value;
		}
		else
		{
			unsigned short value = (m_ScreenColorCellBuffer[offset + 0] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 0] != value;
			m_ScreenColorCellBuffer[offset + 0] = value;

			value = (m_ScreenColorCellBuffer[offset + 1] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 1] != value;
			m_ScreenColorCellBuffer[offset + 1] = value;

			value = (m_ScreenColorCellBuffer[offset + 2] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 2] != value;
			m_ScreenColorCellBuffer[offset + 2] = value;

			value = (m_ScreenColorCellBuffer[offset + 3] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 3] != value;
			m_ScreenColorCellBuffer[offset + 3] = value;
		}

		if (cell_changed)
		{
			m_ScreenDirtyCell.Set(offset + 0);
			m_ScreenDirtyCell.Set(offset + 1);
			m_ScreenDirtyCell.Set(offset + 2);
			m_ScreenDirtyCell.Set(offset + 3);
		}
	}


	void TextField::PrintChar(int inX, int inY, const char inCharacter)
	{
		PrintChar(inX, inY, TextColoring(), inCharacter);
	}


	void TextField::PrintChar(int inX, int inY, const TextColoring& inPrintContext, const char inCharacter)
	{
		const unsigned short current_color_cell_value = inPrintContext.GetColorCellValue();
		const int offset = inY * m_Dimensions.m_Width + inX;
		bool cell_changed = false;

		cell_changed |= m_ScreenCharacterCellBuffer[offset + 0] != inCharacter;
		m_ScreenCharacterCellBuffer[offset + 0] = inCharacter;

		if (inPrintContext.GetChangeBackgroundColor())
		{
			cell_changed |= m_ScreenColorCellBuffer[offset + 0] != current_color_cell_value;
			m_ScreenColorCellBuffer[offset + 0] = current_color_cell_value;
		}
		else
		{
			const unsigned short value = (m_ScreenColorCellBuffer[offset + 0] & 0xff00) | current_color_cell_value;
			cell_changed |= m_ScreenColorCellBuffer[offset + 0] != value;
			m_ScreenColorCellBuffer[offset + 0] = value;
		}

		if (cell_changed)
			m_ScreenDirtyCell.Set(offset + 0);
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------

	void TextField::ReflectToRenderSurface()
	{
		// Prepare cursor
		if (m_Cursor != m_CursorLast)
		{
			auto apply_dirty_region = [this](const Cursor& inCursor)
			{
				for (int y = inCursor.m_Y; y < inCursor.m_Y + inCursor.m_Height; ++y)
				{
					if (y >= 0 && y < m_Dimensions.m_Height)
					{
						for (int x = inCursor.m_X; x < inCursor.m_X + inCursor.m_Width; ++x)
						{
							if (x >= 0 && x < m_Dimensions.m_Width)
							{
								int index = y * m_Dimensions.m_Width + x;
								m_ScreenDirtyCell.Set(index);
							}
						}
					}
				}
			};

			apply_dirty_region(m_Cursor);
			apply_dirty_region(m_CursorLast);
		}

		// Copy new cursor settings to cursor last
		m_CursorLast = m_Cursor;

		// Reflect to screen
		int char_index = 0;
		int out_y = 0;

		int font_width = m_Viewport.GetFont().width;
		int font_height = m_Viewport.GetFont().height;
		int font_pitch = m_Viewport.GetFont().pitch;
		int font_data_size = m_Viewport.GetFont().data_size;
		const unsigned char* font_data = m_Viewport.GetFont().data;

		const Palette& palette = m_Viewport.GetPalette();

		for (int cy = 0; cy < m_Dimensions.m_Height; ++cy)
		{
			int out_x = 0;

			for (int cx = 0; cx < m_Dimensions.m_Width; ++cx)
			{
				if (m_ScreenDirtyCell[char_index])
				{
					unsigned int character_index = static_cast<unsigned int>(m_ScreenCharacterCellBuffer[char_index]) * font_pitch * font_height;

					const bool in_valid_character = (character_index < font_data_size - (font_width * font_pitch));

					const unsigned short character_coloring = m_ScreenColorCellBuffer[char_index];
					const Color ForegroundColor = Color(character_coloring & 0x00ff);
					const Color BackgroundColor = Color(character_coloring >> 8);
					const bool is_cursor = m_Cursor.IsEnabled() && m_Cursor.IsPositionInside(cx, cy);
					const unsigned int color_foreground = !is_cursor ? palette.GetColorARGB(ForegroundColor) : palette.GetColorARGB(BackgroundColor);
					const unsigned int color_background = !is_cursor ? palette.GetColorARGB(BackgroundColor) : palette.GetColorARGB(ForegroundColor);

					for (int i = 0; i < font_height; ++i)
					{
						unsigned int* dest = (unsigned int*)((const char*)m_Surface->pixels + (out_x << 2) + (out_y + i) * m_Surface->pitch);

						for (int j = 0; j < font_pitch; ++j)
						{
							unsigned char data = in_valid_character ? font_data[character_index++] : 0;
							unsigned int pixel_offset = j << 3;

							*(dest + pixel_offset + 0) = data & 0x80 ? color_foreground : color_background;
							*(dest + pixel_offset + 1) = data & 0x40 ? color_foreground : color_background;
							*(dest + pixel_offset + 2) = data & 0x20 ? color_foreground : color_background;
							*(dest + pixel_offset + 3) = data & 0x10 ? color_foreground : color_background;
							*(dest + pixel_offset + 4) = data & 0x08 ? color_foreground : color_background;
							*(dest + pixel_offset + 5) = data & 0x04 ? color_foreground : color_background;
							*(dest + pixel_offset + 6) = data & 0x02 ? color_foreground : color_background;
							*(dest + pixel_offset + 7) = data & 0x01 ? color_foreground : color_background;
						}
					}
				}

				++char_index;

				out_x += font_width;
			}

			out_y += font_height;
		}

		m_ScreenDirtyCell.Clear();
	}
}
