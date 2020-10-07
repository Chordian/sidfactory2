#include "screen_convert.h"

#include "runtime/editor/converters/converterbase.h"
#include "runtime/editor/components/component_text_box.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "utils/utilities.h"

#include <string>
#include <memory>
#include <assert.h>



namespace Editor
{
	ScreenConvert::ScreenConvert(
		Foundation::Viewport* inViewport,
		Foundation::TextField* inMainTextField,
		CursorControl* inCursorControl,
		DisplayState& inDisplayState,
		Utility::KeyHookStore& inKeyHookStore,
		Foundation::IPlatform* inPlatform,
		std::function<void(void)> inExitScreenCallback,
		std::function<bool(ScreenBase*, const std::string&, std::shared_ptr<Utility::C64File>)> inSuccessfullConversionCallback
	)
		: ScreenBase(inViewport, inMainTextField, inCursorControl, inDisplayState, inKeyHookStore)
		, m_Platform(inPlatform)
		, m_ExitScreenCallback(inExitScreenCallback)
		, m_SuccessfullConversionCallback(inSuccessfullConversionCallback)
	{
	}


	void ScreenConvert::PassConverterAndData(
		const std::string& inPathAndFilename,
		std::shared_ptr<ConverterBase> inConverter,
		void* inData,
		unsigned int inDataSize
	)
	{
		m_PathAndFilename = inPathAndFilename;
		m_Converter = inConverter;
		m_Data = inData;
		m_DataSize = inDataSize;
	}


	void ScreenConvert::Activate()
	{
		ScreenBase::Activate();

		// A converter must have been passed along before this screen may activate
		assert(m_Converter != nullptr);
		// Data must exist
		assert(m_Data != nullptr);
		// Data must have a size
		assert(m_DataSize > 0);

		// Clear the text field
		ClearTextField();

		// Activate converter
		m_Converter->Activate(m_Data, m_DataSize, m_Platform, m_ComponentsManager.get());

		const auto& dimensions = m_MainTextField->GetDimensions();
		m_Console = std::make_shared<ComponentTextBox>(0, 0, nullptr, m_MainTextField, 1, 1, dimensions.m_Width - 2, (dimensions.m_Height >> 1) - 2);
		m_ComponentsManager->AddComponent(m_Console);
	}


	void ScreenConvert::Deactivate()
	{
		m_ComponentsManager->Clear();

		m_Converter = nullptr;

		delete[] static_cast<char*>(m_Data);
		m_DataSize = 0;
	}


	void ScreenConvert::TryQuit(std::function<void(bool)> inResponseCallback)
	{
		if (inResponseCallback)
			inResponseCallback(true);
	}


	void ScreenConvert::TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback)
	{
		if (inResponseCallback)
			inResponseCallback(true);
	}


	bool ScreenConvert::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		if (inKeyEvent == SDLK_ESCAPE)
		{
			m_ExitScreenCallback();
			return true;
		}
		if (inKeyEvent == SDLK_RETURN)
		{
			if (m_Converter->GetResult() != nullptr)
			{
				m_SuccessfullConversionCallback(this, m_PathAndFilename, m_Converter->GetResult());
				m_ExitScreenCallback();
			}

			return true;
		}

		return false;
	}


	void ScreenConvert::Update(int inDeltaTick)
	{
		ScreenBase::Update(inDeltaTick);

		ComponentTextBox& cout = *m_Console;

		if (m_Converter != nullptr)
		{
			const auto state_pre = m_Converter->GetState();
			m_Converter->Update();
			const auto state = m_Converter->GetState();

			if (state_pre != state && state == ConverterBase::State::Completed)
			{
				cout << "Conversion has completed!";

				if (m_Converter->GetResult() != nullptr)
					cout << "Successfully... Press ENTER to open into the editor, or ESCAPE to cancel.";
				else
					cout << "Failure.. Press ESCAPE to return to the editor";
			}
		}
	}
}

