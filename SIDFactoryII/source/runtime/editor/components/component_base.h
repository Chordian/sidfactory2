#pragma once

#include "foundation/base/types.h"
#include "runtime/editor/driver/driver_info.h"

#include <functional>

namespace Foundation
{
	class Keyboard;
	class Mouse;
	class TextField;
}

namespace Editor
{
	#pragma warning(disable: 4100)

	class CursorControl;
	class ScreenBase;
	class ComponentsManager;
	class Undo;
	class DisplayState;

	class ComponentBase
	{
	protected:
		ComponentBase() = delete;
		ComponentBase(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight);

	public:
		enum class GetControlType : int
		{
			Default,
			WasSuspended,
			Tabbed_Forward,
			Tabbed_Backward
		};

		virtual ~ComponentBase();

		const int GetComponentID() const;
		const int GetComponentGroupID() const;
		
		Foundation::TextField* GetTextField();

		const Foundation::Point& GetPosition() const;
		const Foundation::Extent& GetDimensions() const;
		const Foundation::Rect& GetRect() const;

		void SetColorRules(const DriverInfo::TableColorRules& inColorRules);

		virtual bool HasDataChange() const;

		virtual bool MayTabOutOfFocus(bool inForward) const;
		virtual bool IsNoteInputSilenced() const;

		virtual void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl);
		virtual void ClearHasControl(CursorControl& inCursorControl);
		virtual bool GetHasControl() const;

		virtual void ForceRefresh();

		virtual bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) = 0;
		virtual bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) = 0;
		virtual void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) = 0;

		virtual void Refresh(const DisplayState& inDisplayState) = 0;
		virtual void HandleDataChange() = 0;
		virtual void PullDataFromSource() = 0;

		virtual void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) = 0;
		virtual void ExecuteAction(int inActionInput) = 0;

		Foundation::Point GetCellPosition(const Foundation::Point& inPixelPosition) const;
		virtual bool ContainsPosition(const Foundation::Point& inPixelPosition) const;
		
	protected:
		Foundation::Point GetLocalCellPosition(const Foundation::Point& inPosition);

		const int m_ComponentID;
		const int m_ComponentGroupID;
		
		Undo* m_Undo;

		Foundation::Point m_Position;
		Foundation::Extent m_Dimensions;
		Foundation::Rect m_Rect;

		Foundation::TextField* m_TextField;
		DriverInfo::TableColorRules m_ColorRules;

		bool m_HasControl;
		bool m_RequireRefresh;
		bool m_HasDataChange;
	};
}