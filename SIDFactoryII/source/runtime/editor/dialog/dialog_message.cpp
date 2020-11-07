#include "dialog_message.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/base/types.h"
#include "foundation/graphics/color.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "utils/usercolors.h"

using namespace Utility;

namespace Editor
{
	using namespace Foundation;

	DialogMessage::DialogMessage(const std::string& inMessage, int inWidth)
		: m_Message(inMessage, inWidth)
		, m_Width(inWidth)
		, m_HasCaption(false)
		, m_CenterMessage(false)
	{

	}


	DialogMessage::DialogMessage(const std::string& inCaption, const std::string& inMessage, int inWidth)
		: m_Caption(inCaption)
		, m_Message(inMessage, inWidth)
		, m_Width(inWidth)
		, m_HasCaption(true)
		, m_CenterMessage(false)
	{

	}


	DialogMessage::DialogMessage(const std::string& inCaption, const std::string& inMessage, int inWidth, bool inCenterMessage)
		: m_Caption(inCaption)
		, m_Message(inMessage, inWidth)
		, m_Width(inWidth)
		, m_HasCaption(true)
		, m_CenterMessage(inCenterMessage)
	{

	}


	DialogMessage::DialogMessage(const std::string& inMessage, int inWidth, bool inCenterMessage, std::function<void(void)>&& inContinue)
		: m_Message(inMessage, inWidth)
		, m_Width(inWidth)
		, m_HasCaption(false)
		, m_CenterMessage(inCenterMessage)
		, m_ContinueFunction(std::move(inContinue))
	{

	}


	DialogMessage::DialogMessage(const std::string& inCaption, const std::string& inMessage, int inWidth, bool inCenterMessage, std::function<void(void)>&& inContinue)
		: m_Caption(inCaption)
		, m_Message(inMessage, inWidth)
		, m_Width(inWidth)
		, m_HasCaption(true)
		, m_CenterMessage(inCenterMessage)
		, m_ContinueFunction(std::move(inContinue))
	{

	}



	DialogMessage::~DialogMessage()
	{

	}


	bool DialogMessage::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		DialogBase::ConsumeInput(inKeyboard, inMouse);

		if (!IsDone())
		{
			for (const auto& keyEvent : inKeyboard.GetKeyEventList())
			{
				switch (keyEvent)
				{
				case SDLK_RETURN:
				case SDLK_ESCAPE:
					m_Done = true;
					m_ContinueFunction();

					return true;
				}
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------------------------------------------

	void DialogMessage::ActivateInternal(Foundation::Viewport* inViewport)
	{
		const int message_height = static_cast<int>(m_Message.GetLines().size());

		m_TextField = inViewport->CreateTextField(2 + m_Width, GetDialogHeight(), 0, 0);
		m_TextField->SetEnable(true);
		m_TextField->SetPositionToCenterOfViewport();

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogBackground));
		m_TextField->PrintAligned({ { 1, GetMessageTopPositionY() }, { m_Width, message_height } }, ToColor(UserColor::DialogText), m_Message, m_CenterMessage ? TextField::HorizontalAlignment::Center : TextField::HorizontalAlignment::Left);

		if (m_HasCaption)
		{
			m_TextField->ColorAreaBackground(ToColor(UserColor::DialogHeader), { {0, 0}, {2 + m_Width, 1} });
			m_TextField->Print((2 + m_Width - static_cast<int>(m_Caption.length())) >> 1, 0, ToColor(UserColor::DialogHeaderText), m_Caption);
		}
	}


	void DialogMessage::DeactivateInternal(Foundation::Viewport* inViewport)
	{
		inViewport->Destroy(m_TextField);
	}

	//-----------------------------------------------------------------------------------------------------------------

	int DialogMessage::GetMessageTopPositionY() const
	{
		return m_HasCaption ? 2 : 1;
	}


	int DialogMessage::GetMessageBottomPositionY() const
	{
		return GetMessageTopPositionY() + static_cast<int>(m_Message.GetLines().size());
	}


	int DialogMessage::GetDialogHeight() const
	{
		return GetMessageBottomPositionY() + 1;
	}
}
