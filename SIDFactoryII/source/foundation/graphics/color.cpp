#include "color.h"
#include "memory.h"
#include <assert.h>

namespace Foundation
{
	Palette::Palette()
	{
		memset(m_Colors, 0, sizeof(m_Colors));

		m_Colors[0x00] = 0x00000000;		// Black
		m_Colors[0x01] = 0xffffffff;		// White
		m_Colors[0x02] = 0xffff0000;		// Red
		m_Colors[0x03] = 0xff00c000;		// Green
		m_Colors[0x04] = 0xff0000ff;		// Blue
		m_Colors[0x05] = 0xffbfbf00;		// Yellow
		m_Colors[0x06] = 0xffff8080;		// Light Red
		m_Colors[0x07] = 0xff40ff40;		// Light Green
		m_Colors[0x08] = 0xff8080ff;		// Light Blue
		m_Colors[0x09] = 0xffffff60;		// Light Yellow
		m_Colors[0x0a] = 0xff800000;		// Dark Red
		m_Colors[0x0b] = 0xff006000;		// Dark Green
		m_Colors[0x0c] = 0xff000080;		// Dark Blue
		m_Colors[0x0d] = 0xff707000;		// Dark Yellow
		m_Colors[0x0e] = 0xffc0c0c0;		// Lighter grey
		m_Colors[0x0f] = 0xff888888;		// Light grey
		m_Colors[0x10] = 0xff707070;		// Grey
		m_Colors[0x11] = 0xff404040;		// Dark Grey
		m_Colors[0x12] = 0xff202020;		// Darker Grey
		m_Colors[0x13] = 0xff600000;		// Darker Red
		m_Colors[0x14] = 0xff004800;		// Darker Green
		m_Colors[0x15] = 0xff000060;		// Darker Blue
		m_Colors[0x16] = 0xff4c4c00;		// Darker Yellow
	}


	void Palette::SetUserColor(unsigned char inUserColorIndex, unsigned int inARGB)
	{
		assert(inUserColorIndex < 0x80);

		if(inUserColorIndex < 0x80)
			m_Colors[0x80 + inUserColorIndex] = inARGB;
	}

	unsigned int Palette::GetColorARGB(Color inColor) const
	{
		unsigned short color_index = static_cast<unsigned short>(inColor);
		if(color_index < 0x100)
			return m_Colors[color_index];

		return 0x00000000;
	}
}
