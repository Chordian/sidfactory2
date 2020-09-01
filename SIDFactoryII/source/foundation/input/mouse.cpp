#include "mouse.h"
#include "SDL.h"

#include <assert.h>

namespace Foundation
{
	Mouse::Mouse()
		: m_ButtonState(0)
		, m_ButtonStateLast(0)
		, m_ButtonStateDoublePress(0)
		, m_TickCounter(0)
		, m_IsCollecting(false)
		, m_IsInsideScreenRect(false)
		, m_WheelDeltaX(0)
		, m_WheelDeltaY(0)
	{
		for (int i = 0; i < Mouse::_Count; ++i)
			m_LastButtonPressedTime[i] = 0;
	}


	void Mouse::BeginCollect(const Rect& inClientRect)
	{
		assert(!m_IsCollecting);

		m_ClientRect = inClientRect;

		m_WheelDeltaX = 0;
		m_WheelDeltaY = 0;

		m_IsCollecting = true;
	}

	void Mouse::EndCollect()
	{
		assert(m_IsCollecting);

		m_IsCollecting = false;
	}

	void Mouse::PushMouseWheelChange(int inDeltaX, int inDeltaY)
	{
		m_WheelDeltaX += inDeltaX;
		m_WheelDeltaY += inDeltaY;
	}


	void Mouse::Update(int inDeltaTick)
	{
		m_TickCounter += inDeltaTick;

		m_ButtonStateLast = m_ButtonState;
		m_ButtonState = SDL_GetMouseState(&m_Position.m_X, &m_Position.m_Y);
		m_IsInsideScreenRect = m_ClientRect.Contains(m_Position);
		m_Position -= m_ClientRect.m_Position;
		
	
		m_ButtonStateDoublePress = 0;

		for (int i = 0; i < Button::_Count; ++i)
		{
			if (IsButtonPressed(static_cast<Button>(i)))
			{
				if (m_TickCounter - m_LastButtonPressedTime[i] < 500)
					m_ButtonStateDoublePress |= 1 << i;

				m_LastButtonPressedTime[i] = m_TickCounter;
			}
		}

	}


	bool Mouse::IsButtonDown(Button inButton) const
	{
		return (m_ButtonState & (1 << inButton)) != 0;
	}

	bool Mouse::IsButtonPressed(Button inButton) const
	{
		return IsButtonDown(inButton) && (m_ButtonStateLast & (1 << inButton)) == 0;
	}

	bool Mouse::IsButtonDoublePressed(Button inButton) const
	{
		return IsButtonPressed(inButton) && (m_ButtonStateDoublePress & (1 << inButton)) != 0;
	}

	Point Mouse::GetPosition() const
	{
		return m_Position;
	}

	Point Mouse::GetWheelDelta() const
	{
		return Point(m_WheelDeltaX, m_WheelDeltaY);
	}

	bool Mouse::IsInsideClientRect() const
	{
		return m_IsInsideScreenRect;
	}
}