#include "dialog_bit_input.h"

#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_check_button.h"
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

	DialogBitInput::DialogBitInput(unsigned char inValue, std::function<void(unsigned char)>&& inDone, std::function<void(void)>&& inCancel)
		: m_Width(0x0b)
		, m_Height(0x09)
		, m_Value(inValue)
		, m_DoneFunction(inDone)
		, m_CancelFunction(inCancel)
	{

	}


	bool DialogBitInput::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
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
				m_Done = true;
				m_DoneFunction(m_Value);
				return true;
			case SDLK_UP:
				m_ComponentsManager->SetTabPreviousComponentFocus();
				return true;
			case SDLK_DOWN:
				m_ComponentsManager->SetNextTabComponentFocus();
				return true;
			}
		}

		return false;
	}


	void DialogBitInput::ActivateInternal(Foundation::Viewport* inViewport)
	{
		m_TextField = inViewport->CreateTextField(m_Width + 2, m_Height + 2, 0, 0);
		m_TextField->SetEnable(true);
		m_TextField->SetPositionToCenterOfViewport();

		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogBackground));
		m_TextField->ColorAreaBackground(ToColor(UserColor::DialogHeader), { {0, 0}, {2 + m_Width, 1} });

		std::string caption = "Edit bits";

		m_TextField->Print((m_Width - static_cast<int>(caption.length())) >> 1, 0, caption);

		m_ComponentsManager = std::make_unique<ComponentsManager>(inViewport, m_CursorControl);

		auto check_button_callback = [&](int inComponentID, bool inChecked)
		{
			if (inChecked)
				m_Value |= inComponentID;
			else
				m_Value &= ~inComponentID;
		};

		auto add_component = [&](unsigned char inValueMask, std::string inString, int inPosX, int inPosY)
		{
			std::shared_ptr<ComponentCheckButton> check_button = std::make_shared<ComponentCheckButton>(inValueMask, 0, nullptr, m_TextField, m_Value & inValueMask, inString, inPosX, inPosY, 5, check_button_callback);
			check_button->SetColors(ToColor(UserColor::DialogText), ToColor(UserColor::DialogBackground));

			m_ComponentsManager->AddComponent(check_button);
		};

		int pos_x = 2;
		int pos_y = 2;

		add_component(0x01, "$01", pos_x, pos_y);
		++pos_y;
		add_component(0x02, "$02", pos_x, pos_y);
		++pos_y;
		add_component(0x04, "$04", pos_x, pos_y);
		++pos_y;
		add_component(0x08, "$08", pos_x, pos_y);
		++pos_y;
		add_component(0x10, "$10", pos_x, pos_y);
		++pos_y;
		add_component(0x20, "$20", pos_x, pos_y);
		++pos_y;
		add_component(0x40, "$40", pos_x, pos_y);
		++pos_y;
		add_component(0x80, "$80", pos_x, pos_y);
		++pos_y;

		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		m_ComponentsManager->SetComponentInFocus(1);
	}


	void DialogBitInput::DeactivateInternal(Foundation::Viewport* inViewport)
	{
		inViewport->Destroy(m_TextField);
	}


	void DialogBitInput::Cancel()
	{
		if(m_CancelFunction)
			m_CancelFunction();
	}
}