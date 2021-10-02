#pragma once

#include "component_base.h"

#include <memory>
#include <string>
#include <functional>

namespace Foundation
{
	class TextField;
	enum class Color : unsigned short;
}

namespace Editor
{
	class CursorControl;
	class ScreenBase;

	class ComponentButton final : public ComponentBase
	{
	public:
		ComponentButton(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, const std::string& inButtonText, int inX, int inY, int inWidth, std::function<void(void)> inButtonPressedCallback);
		~ComponentButton();

		void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;

		void SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

	private:
		bool m_MouseOver;

		Foundation::Color m_BackgroundColor;
		Foundation::Color m_TextColor;

		std::string m_ButtonText;
		std::function<void(void)> m_ButtonPressedCallback; 
	};
}
