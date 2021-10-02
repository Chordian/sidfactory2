#pragma once

#include "component_base.h"

#include "runtime/editor/datasources/idatasource.h"
#include "foundation/graphics/color.h"

#include <memory>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class ComponentListSelector : public ComponentBase
	{
	public:
		ComponentListSelector(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<IDataSource> inDataSource, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight, int inHorizontalMargin, int inVerticalMargin);
		~ComponentListSelector();

		virtual bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		virtual bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		virtual void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		virtual void Refresh(const DisplayState& inDisplayState) override;
		virtual void HandleDataChange() override;
		virtual void PullDataFromSource(const bool inFromUndo) override;

		virtual void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		virtual void ExecuteAction(int inActionInput) override;

		virtual void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;
		virtual void ClearHasControl(CursorControl& inCursorControl) override;

		void SetColors(const Foundation::Color& inBackground, const Foundation::Color& inSelection, const Foundation::Color& inSelectionNoFocus);
		void SetColors(const Foundation::Color& inTextColor);
		
		void SetSelectionIndex(int inSelectionIndex);
		const int GetSelectionIndex() const;

	protected:
		void DoMouseWheel(const Foundation::Mouse& inMouse);
		virtual void RefreshLine(int inIndex, int inPosY);

		void DoCursorDown();
		void DoCursorUp();
		void DoPageDown();
		void DoPageUp();
		void DoPageHome();
		void DoPageEnd();

		std::string CondenseString(const std::string& inString, const char* inPostFix, int inMaxLength) const;

		Foundation::Color m_TextColor;
		Foundation::Color m_BackgroundColor;
		Foundation::Color m_SelectionColor;
		Foundation::Color m_SelectionNoFocusColor;

		int m_HorizontalMargin;
		int m_VerticalMargin;

		int m_ContentX;
		int m_ContentY;
		int m_ContentWidth;
		int m_ContentHeight;

		int m_CursorPos;
		int m_MaxCursorPos;
		int m_TopVisibleIndex;

		std::shared_ptr<IDataSource> m_DataSource;		// Goes to base class
	};
}