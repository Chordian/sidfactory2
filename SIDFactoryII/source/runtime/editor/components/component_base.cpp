#include "foundation/input/keyboard.h"
#include "foundation/graphics/textfield.h"
#include "runtime/editor/cursor_control.h"

#include "component_base.h"


namespace Editor
{
	ComponentBase::ComponentBase(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight)
		: m_ComponentID(inID)
		, m_ComponentGroupID(inGroupID)
		, m_Undo(inUndo)
		, m_TextField(inTextField)
		, m_Position({ inX, inY })
		, m_Dimensions({ inWidth, inHeight })
		, m_Rect({ m_Position, m_Dimensions })
		, m_RequireRefresh(true)
		, m_HasControl(false)
		, m_HasDataChange(false)
	{
	}

	ComponentBase::~ComponentBase()
	{
	}

	const int ComponentBase::GetComponentID() const
	{
		return m_ComponentID;
	}

	const int ComponentBase::GetComponentGroupID() const
	{
		return m_ComponentGroupID;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	Foundation::TextField* ComponentBase::GetTextField()
	{
		return m_TextField;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	const Foundation::Point& ComponentBase::GetPosition() const
	{
		return m_Position;
	}

	const Foundation::Extent& ComponentBase::GetDimensions() const
	{
		return m_Dimensions;
	}

	const Foundation::Rect& ComponentBase::GetRect() const
	{
		return m_Rect;
	}

	Foundation::Point ComponentBase::GetCellPosition(const Foundation::Point& inPixelPosition) const
	{
		Foundation::Point local_position = inPixelPosition - m_TextField->GetPosition();

		const int cell_x = local_position.m_X / Foundation::TextField::font_width;
		const int cell_y = local_position.m_Y / Foundation::TextField::font_height;

		return Foundation::Point({ cell_x, cell_y });
	}

	bool ComponentBase::ContainsPosition(const Foundation::Point& inPixelPosition) const
	{
		return m_Rect.Contains(GetCellPosition(inPixelPosition));
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentBase::SetColorRules(const DriverInfo::TableColorRules& inColorRules)
	{
		m_ColorRules = inColorRules;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentBase::HasDataChange() const
	{
		return m_HasDataChange;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentBase::MayTabOutOfFocus(bool inForward) const
	{
		return true;
	}


	bool ComponentBase::IsNoteInputSilenced() const
	{
		return false;
	}


	void ComponentBase::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		m_HasControl = true;

		// Handle enabling the cursor for the component
		if (m_TextField != nullptr)
			inCursorControl.SetEnabled(true);

		m_RequireRefresh = true;
	}

	void ComponentBase::ClearHasControl(CursorControl& inCursorControl)
	{
		m_HasControl = false;

		if (m_TextField != nullptr)
			inCursorControl.SetEnabled(false);

		m_RequireRefresh = true;
	}

	bool ComponentBase::GetHasControl() const
	{
		return m_HasControl;
	}


	void ComponentBase::ForceRefresh()
	{
		m_RequireRefresh = true;
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	Foundation::Point ComponentBase::GetLocalCellPosition(const Foundation::Point& inPosition)
	{
		const int cell_x = inPosition.m_X / Foundation::TextField::font_width;
		const int cell_y = inPosition.m_Y / Foundation::TextField::font_height;

		return Foundation::Point({ cell_x - m_Position.m_X, cell_y - m_Position.m_Y });
	}
}
