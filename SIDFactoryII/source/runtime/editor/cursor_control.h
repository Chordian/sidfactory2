#pragma once

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class CursorControl final
	{
	public:
		struct Position
		{
			int m_X;
			int m_Y;

			bool operator==(const Position& inOther) const { return inOther.m_X == m_X && inOther.m_Y == m_Y; }
			bool operator!=(const Position& inOther) const { return !(inOther == *this); }
		};

		struct Extent
		{
			int m_Width;
			int m_Height;

			bool operator==(const Extent& inOther) const { return inOther.m_Width == m_Width && inOther.m_Height == m_Height; }
			bool operator!=(const Extent& inOther) const { return !(inOther == *this); }
		};

		CursorControl();

		void SetEnabled(bool inEnabled);
		bool IsEnabled() const;

		void Update(int inTick);

		void SetTargetTextField(Foundation::TextField* inTextField);

		void SetPosition(Position inPosition);
		void SetDimensions(Extent inDimension);

		const Position& GetPosition() const;
		const Extent& GetDimensions() const;

	private:
		bool m_Enabled;
		Foundation::TextField* m_TextField;

		Position m_Position;
		Extent m_Dimensions;

		int m_Tick;

		static const int blink_speed = 9;
	};
}