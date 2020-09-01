#include "display_state.h"

namespace Editor
{
	DisplayState::DisplayState()
		: m_HexIsUppercase(false)
	{
	}


	bool DisplayState::IsHexUppercase() const
	{
		return m_HexIsUppercase;
	}


	void DisplayState::SetIsHexUppercase(bool inUppercase)
	{
		m_HexIsUppercase = inUppercase;
	}
}