#pragma once

#include "foundation/base/types.h"

namespace Foundation
{
	class Mouse
	{
	public:
		enum Button : int
		{
			Left,
			Middle,
			Right,

			_Count
		};

		Mouse(float inScaling);

		void BeginCollect(const Rect& inClientRect);
		void EndCollect();

		void PushMouseWheelChange(int inDeltaX, int inDeltaY);

		void Update(int inDeltaTick);

		bool IsButtonDown(Button inButton) const;
		bool IsButtonPressed(Button inButton) const;
		bool IsButtonDoublePressed(Button inButton) const;

		Point GetPosition() const;
		Point GetWheelDelta() const;

		bool IsInsideClientRect() const;

	private:
		Rect m_ClientRect;
		Point m_Position;
		float m_Scaling;

		bool m_IsCollecting;
		bool m_IsInsideScreenRect;

		int m_WheelDeltaX;
		int m_WheelDeltaY;

		int m_ButtonState;
		int m_ButtonStateLast;
		int m_ButtonStateDoublePress;

		int m_TickCounter;
		int m_LastButtonPressedTime[Button::_Count];
	};
}