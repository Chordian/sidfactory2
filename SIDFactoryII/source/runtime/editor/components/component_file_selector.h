#pragma once

#include "component_list_selector.h"

#include "runtime/editor/datasources/datasource_directory.h"
#include "runtime/editor/datasources/idatasource.h"
#include "foundation/graphics/color.h"


#include <memory>
#include <string>

namespace Foundation
{
	class IPlatform;
	class TextField;
}

namespace Editor
{
	class ComponentFileSelector final : public ComponentListSelector
	{
	public:
		ComponentFileSelector(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<DataSourceDirectory> inDataSource, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight, int inHorizontalMargin, int inVerticalMargin, Foundation::IPlatform* inPlatform);
		~ComponentFileSelector();

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;

	private:
		void RefreshLine(int inIndex, int inPosY) override;
		void ResetCursorPosition();
		void SetCursorPosition(int inCursorPos);

		bool DoMoveToLineWithCharacter(char inCharacter);

		std::shared_ptr<DataSourceDirectory> m_DirectoryDataSource;
		Foundation::IPlatform* m_Platform;
	};
}