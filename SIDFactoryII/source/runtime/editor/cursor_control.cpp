#include "cursor_control.h"
#include "foundation/graphics/textfield.h"

namespace Editor
{
	CursorControl::CursorControl()
		: m_Enabled(false)
		, m_TextField(nullptr)
		, m_Tick(0)
		, m_Position({ 0, 0 })
		, m_Dimensions({ 1, 1 })
	{

	}

	//---------------------------------------------------------------------------------------------------------------------------

	void CursorControl::SetEnabled(bool inEnabled)
	{
		if (inEnabled)
			m_Tick = 0;										// If enabled, reset the tick count, so that the cursor will become visiblie on next update.
		else if (m_TextField != nullptr)
			m_TextField->GetCursor().SetEnabled(false);		// If disabled, makes sure the cursor is turned of in the text field

		m_Enabled = inEnabled;
	}


	bool CursorControl::IsEnabled() const
	{
		return m_Enabled;
	}

	//---------------------------------------------------------------------------------------------------------------------------

	void CursorControl::Update(int inTick)
	{
		if (m_Enabled)
		{
			if (m_TextField != nullptr)
			{
				m_TextField->GetCursor().SetPosition(m_Position.m_X, m_Position.m_Y);
				m_TextField->GetCursor().SetDimensions(m_Dimensions.m_Width, m_Dimensions.m_Height);
				m_TextField->GetCursor().SetEnabled(m_Tick & (1 << (blink_speed - 1)));
			}

			m_Tick -= inTick;

			if (m_Tick < 0)
				m_Tick += 1 << blink_speed;
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------

	void CursorControl::SetTargetTextField(Foundation::TextField* inTextField)
	{
		if (inTextField != m_TextField)
		{
			if (m_TextField != nullptr)
				m_TextField->GetCursor().SetEnabled(false);

			m_TextField = inTextField;

			m_Tick = 0;
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------

	void CursorControl::SetPosition(CursorControl::Position inPosition)
	{
		if (inPosition != m_Position)
		{
			m_Position = inPosition;
			m_Tick = 0;
		}
	}

	void CursorControl::SetDimensions(CursorControl::Extent inDimension)
	{
		if (inDimension != m_Dimensions)
		{
			m_Dimensions = inDimension;
			m_Tick = 0;
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------

	const CursorControl::Position& CursorControl::GetPosition() const
	{
		return m_Position;
	}

	const CursorControl::Extent& CursorControl::GetDimensions() const
	{
		return m_Dimensions;
	}
}