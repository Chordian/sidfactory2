#pragma once

#include "component_list_selector.h"

#include "runtime/editor/datasources/datasource_tlist.h"
#include "runtime/editor/datasources/idatasource.h"
#include "foundation/graphics/color.h"

#include <memory>
#include <string>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class ComponentStringListSelector final : public ComponentListSelector
	{
	public:
		ComponentStringListSelector(
			int inID, 
			int inGroupID, 
			Undo* inUndo, 
			std::shared_ptr<DataSourceTList<std::string>> inDataSource, 
			Foundation::TextField* inTextField, 
			int inX, 
			int inY, 
			int inWidth, 
			int inHeight, 
			int inHorizontalMargin, 
			int inVerticalMargin);
		~ComponentStringListSelector();

		void SetSelectionCallback(const std::function<void(bool)>& inOnSelectionCallback);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;

	private:
		void RefreshLine(int inIndex, int inPosY) override;

		std::shared_ptr<DataSourceTList<std::string>> m_DataSourceStringList;
		std::function<void(bool)> m_OnSelection;
	};
}