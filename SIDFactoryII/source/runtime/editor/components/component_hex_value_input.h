#pragma once

#include "component_base.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"

#include <memory>

namespace Foundation
{
	class TextField;
	enum class Color : unsigned short;
}

namespace Editor
{
	class ScreenBase;

	class ComponentHexValueInput final : public ComponentBase
	{
	public:
		ComponentHexValueInput(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<DataSourceMemoryBuffer> inDataSource, Foundation::TextField* inTextField, int inDigitCount, int inX, int inY);
		~ComponentHexValueInput();

		void SetColors(const Foundation::Color& inTextColor);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;

		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;

	private:
		void ApplyCharacter(char inCharacter);
		void ApplyCursorPosition(CursorControl& inCursorControl);

		void DoCursorForward();
		void DoCursorBackwards();

		int m_CursorPos;
		int m_MaxCursorPos;
		int m_DigitCount;

		Foundation::Color m_TextColor;

		std::shared_ptr<DataSourceMemoryBuffer> m_DataSource;
	};
}
