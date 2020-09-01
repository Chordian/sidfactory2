#include "component_file_selector.h"
#include "runtime/editor/cursor_control.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/dialog/dialog_message_yesno.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "foundation/graphics/textfield.h"
#include "foundation/platform/iplatform.h"

#include <cctype>

using namespace Foundation;

namespace Editor
{
	ComponentFileSelector::ComponentFileSelector(int inID, int inGroupID, Undo* inUndo, std::shared_ptr<DataSourceDirectory> inDataSource, Foundation::TextField* inTextField, int inX, int inY, int inWidth, int inHeight, int inHorizontalMargin, int inVerticalMargin, IPlatform* inPlatform)
		: ComponentListSelector(inID, inGroupID, inUndo, inDataSource, inTextField, inX, inY, inWidth, inHeight, inHorizontalMargin, inVerticalMargin)
		, m_DirectoryDataSource(inDataSource)
		, m_Platform(inPlatform)
	{
		ResetCursorPosition();
	}

	ComponentFileSelector::~ComponentFileSelector()
	{

	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentFileSelector::ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl)
		{
			if (ComponentListSelector::ConsumeInput(inKeyboard, inCursorControl, inComponentsManager))
				return true;

			for (auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_BACKSPACE:
					m_DirectoryDataSource->Back();
					ResetCursorPosition();
					m_RequireRefresh = true;

					return true;
				case SDLK_RETURN:
					{
						const auto result = m_DirectoryDataSource->Select(m_CursorPos);

						if(result == DataSourceDirectory::SelectResult::SelectFolderSucceeded)
							ResetCursorPosition();
						if (result == DataSourceDirectory::SelectResult::SelectFolderFailed)
							inComponentsManager.StartDialog(std::make_shared<DialogMessage>("Error", "The filesystem prevented access to the selected folder!", 100, true, []() {}));
					}

					m_RequireRefresh = true;

					return true;
				case SDLK_DELETE:
					{
						const auto& directory_entry = (*m_DirectoryDataSource)[m_CursorPos];

						if (directory_entry.m_Type == DirectoryEntry::File)
						{
							ComponentsManager* components_manager = &inComponentsManager;
							inComponentsManager.StartDialog(std::make_shared<DialogMessageYesNo>("Delete file", "Do you want to delete the file:\n" + directory_entry.m_Path.string() + "?\nYou cannot undo this", 120, [&, directory_entry, components_manager]()
								{
									if (m_Platform->Storage_DeleteFile(directory_entry.m_Path.string()))
									{
										m_DirectoryDataSource->GenerateData();
										SetCursorPosition(m_CursorPos > 0 ? (m_CursorPos - 1) : 0);
										m_RequireRefresh = true;
									}
									else
									{
										components_manager->StartDialog(std::make_shared<DialogMessage>("Error", "Deleting file failed!", 100, true, []() {}));
									}
								}, []() {}));
						}
					}
				}

				char character = KeyboardUtils::FilterLetter(key_event);

				if (character == 0)
					character = KeyboardUtils::FilterDecimalDigit(key_event);

				if (character != 0)
				{
					if (DoMoveToLineWithCharacter(character))
						m_RequireRefresh = true;

					return true;
				}
			}
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	bool ComponentFileSelector::ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (inMouse.IsButtonPressed(Mouse::Left))
		{
			Foundation::Point local_cell_position = GetLocalCellPosition(inMouse.GetPosition());

			if (local_cell_position.m_X >= m_VerticalMargin && local_cell_position.m_X < m_Dimensions.m_Width - m_VerticalMargin && local_cell_position.m_Y >= 0 && local_cell_position.m_Y < m_Dimensions.m_Height)
			{
				int cursor_pos = m_TopVisibleIndex + local_cell_position.m_Y - m_VerticalMargin;

				if (cursor_pos >= 0 && cursor_pos < m_DataSource->GetSize())
				{
					m_CursorPos = cursor_pos;
					m_RequireRefresh = true;

					if (inMouse.IsButtonDoublePressed(Mouse::Left))
					{
						const auto result = m_DirectoryDataSource->Select(m_CursorPos);

						if (result == DataSourceDirectory::SelectResult::SelectFolderSucceeded)
							ResetCursorPosition();
						if (result == DataSourceDirectory::SelectResult::SelectFolderFailed)
							inComponentsManager.StartDialog(std::make_shared<DialogMessage>("Error", "The filesystem prevented access to the selected folder!", 100, true, []() {}));
					}
				}

				return true;
			}
		}

		DoMouseWheel(inMouse);

		return false;
	}


	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentFileSelector::RefreshLine(int inIndex, int inPosY)
	{
		const int max_name_length = m_ContentWidth - 11;
		const DirectoryEntry& directory_entry = (*m_DirectoryDataSource)[inIndex];

        std::string name = [&]() -> std::string
		{
			if (directory_entry.m_Type != DirectoryEntry::Drive)
				return directory_entry.m_Path.filename().string();

			return directory_entry.m_Path.string();
		}();

		if (static_cast<int>(name.length()) > max_name_length)
			name = CondenseString(name, "...", max_name_length);

		m_TextField->Print(m_ContentX, inPosY, m_TextColor, name);

		if (directory_entry.m_Type == DirectoryEntry::Drive)
			m_TextField->Print(m_ContentX + m_ContentWidth - 10, inPosY, m_TextColor, "<DRIVE>");
		else if (directory_entry.m_Type == DirectoryEntry::Folder || directory_entry.m_Type == DirectoryEntry::Back)
			m_TextField->Print(m_ContentX + m_ContentWidth - 10, inPosY, m_TextColor, "<FOLDER>");
	}

	//----------------------------------------------------------------------------------------------------------------------------------------

	void ComponentFileSelector::ResetCursorPosition()
	{
		int cursor_pos = 0;
		auto& data_source = *m_DirectoryDataSource;

		if (data_source.HasFileSelection())
		{
			cursor_pos = data_source.GetFileSelectIndex();
			data_source.ClearFileSelection();
		}
		else
		{
			for (int i = 0; i < m_DataSource->GetSize(); ++i)
			{
				const auto type = (*m_DirectoryDataSource)[i].m_Type;
				if (type != DirectoryEntry::Drive && type != DirectoryEntry::Back)
				{
					cursor_pos = i;
					break;
				}
			}
		}

		SetCursorPosition(cursor_pos);
	}

	void ComponentFileSelector::SetCursorPosition(int inCursorPos)
	{
		m_CursorPos = inCursorPos;

		if (m_CursorPos >= m_ContentHeight)
			m_TopVisibleIndex = m_CursorPos - m_ContentHeight + 1;
		else
			m_TopVisibleIndex = 0;
	}

	bool ComponentFileSelector::DoMoveToLineWithCharacter(char inCharacter)
	{
		const int size = m_DirectoryDataSource->GetSize();

		int index = m_CursorPos;

		for (int i = 0; i < size; ++i)
		{
			++index;
			if (index >= size)
				index = 0;

			const DirectoryEntry& directory_entry = (*m_DirectoryDataSource)[index];

			if (directory_entry.m_Type == DirectoryEntry::File || directory_entry.m_Type == DirectoryEntry::Folder)
			{
				const auto filename = directory_entry.m_Path.filename();
				if (std::tolower(filename.c_str()[0]) == inCharacter)
				{
					m_CursorPos = index;

					if (m_CursorPos < m_TopVisibleIndex)
						m_TopVisibleIndex = m_CursorPos;
					else if (m_CursorPos >= m_TopVisibleIndex + (m_ContentHeight - 1))
						m_TopVisibleIndex = m_CursorPos - (m_ContentHeight - 1);

					return true;
				}
			}
		}

		return false;
	}
}

