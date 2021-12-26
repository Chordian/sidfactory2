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

	class ComponentConsole final : public ComponentBase
	{
	public:
		ComponentConsole(int inID, int inGroupID, Undo* inUndo, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight);
		~ComponentConsole();

		ComponentConsole& operator << (const char* inString);
		ComponentConsole& operator << (std::string& inText);
		ComponentConsole& operator << (unsigned int inValue);
		ComponentConsole& operator << (int inValue);


		void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;
		void SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

	private:
		bool m_MouseOver;

		Foundation::Color m_BackgroundColor;
		Foundation::Color m_TextColor;

		std::vector<std::string> m_TextLines;
	};
}
