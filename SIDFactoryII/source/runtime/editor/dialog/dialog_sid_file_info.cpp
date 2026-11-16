#include "dialog_sid_file_info.h"

#include "runtime/editor/components_manager.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/base/types.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "utils/usercolors.h"
#include "utils/sidpanning.h"

using namespace Utility;

namespace Editor
{
	using namespace Foundation;

	namespace
	{
		const int OptionButtonX = 13;
		const int OptionButtonWidth = 16;
		const int OptionFirstRow = 6;

		// Left aligned and padded: a shorter text overwrites the longer one before it
		std::string PadOptionText(const std::string& inText)
		{
			std::string text = inText.substr(0, OptionButtonWidth);
			text.resize(OptionButtonWidth, ' ');

			return text;
		}
	}


	DialogSIDFileInfo::DialogSIDFileInfo(std::function<void(std::string, std::string, std::string)>&& inDone, std::function<void(void)>&& inCancel)
		: m_Width(0x2d)
		, m_Height(6)
		, m_MessageMaxLength(0x20)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
		, m_SIDCount(1)
	{

	}


	DialogSIDFileInfo::DialogSIDFileInfo(
		unsigned int inSIDCount,
		const ExportOptions& inExportOptions,
		std::function<void(std::string, std::string, std::string, ExportOptions)>&& inDone,
		std::function<void(void)>&& inCancel)
		: m_Width(0x2d)
		, m_Height(6)
		, m_MessageMaxLength(0x20)
		, m_DoneWithOptionsFunction(inDone)
		, m_CancelFunction(inCancel)
		, m_SIDCount(inSIDCount)
		, m_ExportOptions(inExportOptions)
	{
		// PSID v4 holds the address of three SIDs at most
		if (m_SIDCount > 3)
			m_ExportOptions.m_Version5 = true;

		// Format, layout, mode, result and a blank row
		if (HasExportOptions())
			m_Height += 5;
	}


	bool DialogSIDFileInfo::HasExportOptions() const
	{
		return m_SIDCount > 1;
	}


	void DialogSIDFileInfo::Cancel()
	{
		if (m_CancelFunction)
			m_CancelFunction();
	}


	bool DialogSIDFileInfo::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		DialogBase::ConsumeInput(inKeyboard, inMouse);

		for (const auto& key_event : inKeyboard.GetKeyEventList())
		{
			switch (key_event)
			{
			case SDLK_ESCAPE:
				m_Done = true;
				m_CancelFunction();

				return true;
			case SDLK_RETURN:
				// The option buttons cycle their value on ENTER and keep the focus, TAB moves on
				if (HasExportOptions()
					&& (m_ComponentsManager->IsComponentInFocus(m_ComponentButtonFormat->GetComponentID())
						|| m_ComponentsManager->IsComponentInFocus(m_ComponentButtonPanLayout->GetComponentID())
						|| m_ComponentsManager->IsComponentInFocus(m_ComponentButtonPanMode->GetComponentID())))
					return true;

				if (!m_ComponentsManager->IsComponentInFocus(m_ComponentButtonOk->GetComponentID()))
					m_ComponentsManager->SetNextTabComponentFocus();

				return true;
			}
		}

		return false;
	}


	void DialogSIDFileInfo::ActivateInternal(Foundation::Viewport* inViewport)
	{
		m_TextField = inViewport->CreateTextField(2 + m_Width, 2 + m_Height, 0, 0);
		m_TextField->SetEnable(true);
		m_TextField->SetPositionToCenterOfViewport();

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogBackground));

		std::string caption = "SID file info";

		Color text_color = ToColor(UserColor::DialogText);

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogHeader), { {0, 0}, {2 + m_Width, 1} });
		m_TextField->Print((m_Width - static_cast<int>(caption.length())) >> 1, 0, ToColor(UserColor::DialogHeaderText), caption);
		m_TextField->Print(2, 2, text_color, "Title    :");
		m_TextField->Print(2, 3, text_color, "Author   :");
		m_TextField->Print(2, 4, text_color, "Copyright:");

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		// m_ComponentsManager->SetGroupEnabledForTabbing(0);

		m_TextDataBufferTitle = std::make_shared<DataSourceMemoryBuffer>(m_MessageMaxLength);
		m_TextDataBufferAuthor = std::make_shared<DataSourceMemoryBuffer>(m_MessageMaxLength);
		m_TextDataBufferCopyright = std::make_shared<DataSourceMemoryBuffer>(m_MessageMaxLength);
		m_TextInputComponentTitle = std::make_shared<ComponentTextInput>
			(
				0, 0,
				nullptr,
				m_TextDataBufferTitle,
				m_TextField,
				ToColor(UserColor::DialogText),
				13,
				2,
				m_MessageMaxLength,
				false
			);
		m_ComponentsManager->AddComponent(m_TextInputComponentTitle);
		m_TextInputComponentAuthor = std::make_shared<ComponentTextInput>
			(
				1, 0,
				nullptr,
				m_TextDataBufferAuthor,
				m_TextField,
				ToColor(UserColor::DialogText),
				13,
				3,
				m_MessageMaxLength,
				false
			);
		m_ComponentsManager->AddComponent(m_TextInputComponentAuthor);
		m_TextInputComponentCopyright = std::make_shared<ComponentTextInput>
			(
				2, 0,
				nullptr,
				m_TextDataBufferCopyright,
				m_TextField,
				ToColor(UserColor::DialogText),
				13,
				4,
				m_MessageMaxLength,
				false
			);
		m_ComponentsManager->AddComponent(m_TextInputComponentCopyright);

		if (HasExportOptions())
		{
			m_TextField->Print(2, OptionFirstRow, text_color, "Format   :");
			m_TextField->Print(2, OptionFirstRow + 1, text_color, "Panning  :");
			m_TextField->Print(2, OptionFirstRow + 2, text_color, "Pan mode :");
			m_TextField->Print(2, OptionFirstRow + 3, text_color, "Stereo   :");

			// PSID v3/v4 has no panning: the setting then holds for the reSID output only
			m_ComponentButtonFormat = std::make_shared<ComponentButton>(3, 0,
				nullptr,
				m_TextField, PadOptionText(""),
				OptionButtonX, OptionFirstRow,
				OptionButtonWidth,
				[&]()
				{
					if (m_SIDCount <= 3)
						m_ExportOptions.m_Version5 = !m_ExportOptions.m_Version5;

					UpdateExportOptionTexts();
				});
			m_ComponentsManager->AddComponent(m_ComponentButtonFormat);

			m_ComponentButtonPanLayout = std::make_shared<ComponentButton>(4, 0,
				nullptr,
				m_TextField, PadOptionText(""),
				OptionButtonX, OptionFirstRow + 1,
				OptionButtonWidth,
				[&]()
				{
					m_ExportOptions.m_PanLayout = static_cast<SIDPanLayout>((static_cast<int>(m_ExportOptions.m_PanLayout) + 1) & 0x03);
					UpdateExportOptionTexts();
				});
			m_ComponentsManager->AddComponent(m_ComponentButtonPanLayout);

			m_ComponentButtonPanMode = std::make_shared<ComponentButton>(5, 0,
				nullptr,
				m_TextField, PadOptionText(""),
				OptionButtonX, OptionFirstRow + 2,
				OptionButtonWidth,
				[&]()
				{
					m_ExportOptions.m_PanMode = static_cast<SIDPanMode>((static_cast<int>(m_ExportOptions.m_PanMode) + 1) & 0x03);
					UpdateExportOptionTexts();
				});
			m_ComponentsManager->AddComponent(m_ComponentButtonPanMode);

			UpdateExportOptionTexts();
		}

		const int button_width = 10;

		m_ComponentButtonOk = std::make_shared<ComponentButton>(6, 0, 
			nullptr,
			m_TextField, "Ok",
			(m_Width - button_width) >> 1, m_Height,
			button_width,
			[&]() { OnDone(); });
		m_ComponentsManager->AddComponent(m_ComponentButtonOk);

		m_ComponentsManager->SetComponentInFocus(m_TextInputComponentTitle);
	}


	void DialogSIDFileInfo::DeactivateInternal(Foundation::Viewport* inViewport)
	{
		inViewport->Destroy(m_TextField);
	}


	void DialogSIDFileInfo::OnDone()
	{
		m_Done = true;

		const std::string title = ConvertToString(m_TextDataBufferTitle);
		const std::string author = ConvertToString(m_TextDataBufferAuthor);
		const std::string copyright = ConvertToString(m_TextDataBufferCopyright);

		if (m_DoneWithOptionsFunction)
			m_DoneWithOptionsFunction(title, author, copyright, m_ExportOptions);
		else
			m_DoneFunction(title, author, copyright);
	}


	void DialogSIDFileInfo::UpdateExportOptionTexts()
	{
		m_ComponentButtonFormat->SetText(PadOptionText(m_ExportOptions.m_Version5 ? "SID v5" : (m_SIDCount == 2 ? "PSID v3" : "PSID v4")));
		m_ComponentButtonPanLayout->SetText(PadOptionText(GetSIDPanLayoutName(m_ExportOptions.m_PanLayout)));
		m_ComponentButtonPanMode->SetText(PadOptionText(GetSIDPanModeName(m_ExportOptions.m_PanMode)));

		SIDPan pan[SIDPanMaxSIDCount];
		ComputeSIDPanning(m_ExportOptions.m_PanLayout, m_ExportOptions.m_PanMode, m_SIDCount, pan);

		std::string stereo_text = SIDPanningToString(pan, m_SIDCount) + (m_ExportOptions.m_Version5 ? "" : " (reSID only)");
		stereo_text.resize(static_cast<size_t>(m_Width - OptionButtonX), ' ');

		m_TextField->Print(OptionButtonX, OptionFirstRow + 3, ToColor(UserColor::DialogText), stereo_text);
	}


	std::string DialogSIDFileInfo::ConvertToString(std::shared_ptr<DataSourceMemoryBuffer>& inMemoryBuffer)
	{
		const int text_length = [&]()
		{
			const int buffer_size = inMemoryBuffer->GetSize();

			for (int i = 0; i < buffer_size; ++i)
			{
				if ((*inMemoryBuffer)[i] == 0)
					return i;
			}

			return buffer_size;
		}();

		if (text_length > 0)
		{
			char* text_buffer = new char[text_length];

			for (int i = 0; i < text_length; ++i)
				text_buffer[i] = static_cast<char>((*inMemoryBuffer)[i]);

			std::string return_string(text_buffer, text_length);
			delete[] text_buffer;

			return return_string;
		}

		return std::string();
	}
}