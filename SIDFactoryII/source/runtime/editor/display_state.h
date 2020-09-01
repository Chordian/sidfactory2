#pragma once

namespace Editor
{
	class DisplayState
	{
	public:
		DisplayState();

		bool IsHexUppercase() const;
		void SetIsHexUppercase(bool inIsUppercase);

	private:
		bool m_HexIsUppercase;
	};
}