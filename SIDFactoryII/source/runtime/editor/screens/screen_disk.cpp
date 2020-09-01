#include "screen_disk.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/keyboard.h"
#include "foundation/platform/iplatform.h"

#include "runtime/editor/cursor_control.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/datasources/datasource_directory.h"
#include "runtime/editor/components/component_file_selector.h"
#include "runtime/editor/editor_types.h"
#include "utils/usercolors.h"

#include <vector>
#include <algorithm>
#include <cctype>
#include <memory>
#include <assert.h>
#include "libraries/ghc/fs_std.h"


using namespace Foundation;
using namespace fs;
using namespace Utility;

namespace Editor
{
	ScreenDisk::ScreenDisk(
		Foundation::IPlatform* inPlatform,
		Foundation::Viewport* inViewport,
		Foundation::TextField* inMainTextField,
		CursorControl* inCursorControl,
		DisplayState& inDisplayState,
		Utility::KeyHookStore& inKeyHookStore,
		std::function<void(const std::string&, FileType)> inSelectionCallback,
		std::function<void(void)> inCancelCallback)
		: ScreenBase(inViewport, inMainTextField, inCursorControl, inDisplayState, inKeyHookStore)
		, m_Platform(inPlatform)
		, m_Mode(Mode::Load)
		, m_SelectionCallback(inSelectionCallback)
		, m_CancelCallback(inCancelCallback)
	{
	}

	ScreenDisk::~ScreenDisk()
	{
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenDisk::Activate()
	{
		assert(m_Platform != nullptr);
		ScreenBase::Activate();

		// Clear the text field
		ClearTextField();

		// Prepare the screen layout
		PrepareLayout();
	}

	void ScreenDisk::Deactivate()
	{
		m_ComponentsManager->Clear();
		m_DataSourceDirectory = nullptr;
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenDisk::TryQuit(std::function<void(bool)> inResponseCallback)
	{
		ShowQuitDialog(m_ComponentsManager, inResponseCallback);
	}

	void ScreenDisk::TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback)
	{
		ShowTryLoadDialog(inPathAndFilename, m_ComponentsManager, inResponseCallback);
	}

	//------------------------------------------------------------------------------------------------------------

	bool ScreenDisk::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		switch (inKeyEvent)
		{
		case SDLK_ESCAPE:
			m_CancelCallback();
			return true;
		case SDLK_RETURN:
		{
			std::string file_name = m_DataSourceFileNameBuffer->GetAsString();

			path cur_path = current_path();
			path path_and_filename = cur_path / file_name;

			std::string extension = path_and_filename.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](char c) { return std::tolower(c); });

			FileType file_type = FileType::Unknown;
			
			if (m_Mode == Mode::SavePacked)
			{
				if (extension == ".sid")
					file_type = FileType::SID;
				else
				{
					file_type = FileType::PRG;
					path_and_filename.replace_extension(".prg");
				}
			}
			else if (m_Mode == Mode::Save)
			{
				file_type = FileType::SF2;
				path_and_filename.replace_extension(".sf2");
			}
			else if (m_Mode == Mode::SaveInstrument)
			{
				file_type = FileType::SI2;
				path_and_filename.replace_extension(".si2");
			}
			else if (m_Mode == Mode::Load)
				file_type = FileType::SF2;
			else if (m_Mode == Mode::LoadInstrument)
				file_type = FileType::SI2;
				

			m_SelectionCallback(path_and_filename.string(), file_type);
		}
		return true;
		}

		return false;
	}


	void ScreenDisk::Update(int inDeltaTick)
	{
		ScreenBase::Update(inDeltaTick);

		// This will probably have to move elsewhere.. The save variant of the screen should probably be an overload of the load screen!
		if (m_DataSourceDirectory->HasFileSelection())
		{
			if (m_Mode == Mode::Load || m_Mode == Mode::Import)
			{
				auto selection = m_DataSourceDirectory->GetFileSelection();
				m_DataSourceDirectory->ClearFileSelection();
				m_SelectionCallback(selection.m_Path.string(), FileType::SF2);
			}
			if (m_Mode == Mode::LoadInstrument)
			{
				auto selection = m_DataSourceDirectory->GetFileSelection();
				m_DataSourceDirectory->ClearFileSelection();
				m_SelectionCallback(selection.m_Path.string(), FileType::SI2);
			}
			else if (m_Mode == Mode::Save || m_Mode == Mode::SavePacked || m_Mode == Mode::SaveInstrument)
			{
				const std::string file_name = m_DataSourceDirectory->GetFileSelection().m_Path.filename().string();
				*m_DataSourceFileNameBuffer = file_name;
				m_DataSourceDirectory->ClearFileSelection();
				m_ComponentFileNameInput->SetCursorPosition(static_cast<int>(file_name.length()));
				m_ComponentsManager->SetComponentInFocus(m_ComponentFileNameInput);
			}
		}
	}

	void ScreenDisk::Refresh()
	{
		ScreenBase::Refresh();

		m_ComponentsManager->Refresh(m_DisplayState);
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenDisk::SetMode(Mode inMode)
	{
		m_Mode = inMode;
	}

	const ScreenDisk::Mode ScreenDisk::GetMode() const
	{
		return m_Mode;
	}

	void ScreenDisk::SetSuggestedFileName(const std::string& inSuggestedFileName)
	{
		path file_name = inSuggestedFileName;
		m_SuggestedFileName = file_name.extension().string() == ".sf2" ? file_name.filename().string() : "";
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenDisk::PrepareLayout()
	{
		const int horizontal_margin = 5;
		const int top_margin = 4;
		const int bottom_margin = 6;

		Foundation::Extent dimensions = m_MainTextField->GetDimensions();

		m_DataSourceDirectory = std::make_shared<DataSourceDirectory>(m_Platform);

		auto file_selector = std::make_shared<ComponentFileSelector>(
			0, 0,
			nullptr,
			m_DataSourceDirectory,
			m_MainTextField,
			horizontal_margin,
			top_margin,
			dimensions.m_Width - (horizontal_margin << 1),
			dimensions.m_Height - (top_margin + bottom_margin),
			1,
			1,
			m_Platform
		);

		Color selection_color = [&]()
		{
			if (m_Mode == Mode::Load)
				return ToColor(UserColor::FileSelectorCursorFocusLoadSong);
			if (m_Mode == Mode::Save)
				return ToColor(UserColor::FileSelectorCursorFocusSaveSong); 
			if (m_Mode == Mode::Import)
				return ToColor(UserColor::FileSelectorCursorFocusImportSong);
			if (m_Mode == Mode::SavePacked)
				return ToColor(UserColor::FileSelectorCursorFocusSavePacked);

			// Undefined
			return Color::Black;
		}();

		file_selector->SetColors(ToColor(UserColor::FileSelectorBackground), selection_color, ToColor(UserColor::FileSelectorCursorNoFocus));
		file_selector->SetColors(ToColor(UserColor::FileSelectorListText));

		m_ComponentsManager->AddComponent(file_selector);

		const int filename_position_y = dimensions.m_Height - bottom_margin + 1;
		const int filename_position_x = horizontal_margin;
		const int max_filename_size = 64;

		m_MainTextField->Print(filename_position_x, filename_position_y, ToColor(UserColor::FileSelectorTextInput), "Filename: ");

		m_DataSourceFileNameBuffer = std::make_shared<DataSourceMemoryBufferString>(max_filename_size);
		m_ComponentFileNameInput = std::make_shared<ComponentTextInput>(
			1, 0,
			nullptr,
			m_DataSourceFileNameBuffer,
			m_MainTextField,
			ToColor(UserColor::FileSelectorTextInput),
			filename_position_x + 10,
			filename_position_y,
			max_filename_size,
			true);

		if (m_Mode == Mode::Save && m_SuggestedFileName.length() > 0)
		{
			*m_DataSourceFileNameBuffer = m_SuggestedFileName;
			m_DataSourceDirectory->ClearFileSelection();
			m_ComponentFileNameInput->SetCursorPosition(static_cast<int>(m_SuggestedFileName.length()));
		}

		m_ComponentsManager->AddComponent(m_ComponentFileNameInput);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		//m_ComponentsManager->SetGroupEnabledForTabbing(0);

		if (m_Mode == Mode::Load || m_Mode == Mode::LoadInstrument || m_Mode == Mode::Import)
			m_ComponentsManager->SetComponentInFocus(0);
		else
			m_ComponentsManager->SetComponentInFocus(m_ComponentFileNameInput);

		std::string headline = [&]()
		{
			if (m_Mode == Mode::Load)
				return "Load file";
			if (m_Mode == Mode::Save)
				return "Save file";
			if (m_Mode == Mode::Import)
				return "Import song data";
			if (m_Mode == Mode::LoadInstrument)
				return "Load instrument file";
			if (m_Mode == Mode::SaveInstrument)
				return "Save instrument file";
			if (m_Mode == Mode::SavePacked)
				return "Save packed file";

			return "Unknown";
		}();

		m_MainTextField->Print(horizontal_margin, 2, std::string(headline));
	}
}
