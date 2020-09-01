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

	class ComponentTextInput final : public ComponentBase
	{
	public:
		ComponentTextInput(
			int inID, 
			int inGroupID, 
			Undo* inUndo, 
			std::shared_ptr<DataSourceMemoryBuffer> inDataSource, 
			Foundation::TextField* inTextField, 
			Foundation::Color inColor,
			int inX, 
			int inY, 
			int inWidth,
			bool inIsForFilename);
		~ComponentTextInput();

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		
		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource() override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

		void SetCursorPosition(int inCursorPosition);

	private:
		void ApplyCharacter(char inCharacter);
		void ApplyCursorPosition(CursorControl& inCursorControl);

		void DoCursorForward();
		void DoCursorBackwards();
		void DoInsert();
		void DoDelete();
		void DoBackspace(bool inIsShiftDown);

		const int GetMaxPossibleCursorPosition() const;

		int m_CursorPos;
		int m_MaxCursorPos;
		bool m_IsForFilename;

		Foundation::Color m_TextColor;

		std::shared_ptr<DataSourceMemoryBuffer> m_DataSource;
	};
}
