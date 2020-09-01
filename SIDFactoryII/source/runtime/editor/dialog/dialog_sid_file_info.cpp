#include "dialog_sid_file_info.h"

#include "runtime/editor/components_manager.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/types.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "utils/usercolors.h"

using namespace Utility;

namespace Editor
{
	using namespace Foundation;

	DialogSIDFileInfo::DialogSIDFileInfo(std::function<void(std::string, std::string, std::string)>&& inDone, std::function<void(void)>&& inCancel)
		: m_Width(0x2d)
		, m_Height(6)
		, m_MessageMaxLength(0x20)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
	{

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
		// m_TextField->PrintAligned({ { 1, GetMessageTopPositionY() }, { m_Width, message_height } }, m_Message, m_CenterMessage ? TextField::HorizontalAlignment::Center : TextField::HorizontalAlignment::Left);

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

		const int button_width = 10;

		m_ComponentButtonOk = std::make_shared<ComponentButton>(3, 0, 
			nullptr,
			m_TextField, "Ok",
			(m_Width - button_width) >> 1, 6,
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

		m_DoneFunction(title, author, copyright);
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