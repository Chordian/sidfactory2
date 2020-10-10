#include "screen_convert.h"

#include "runtime/editor/screens/statusbar/status_bar.h"
#include "runtime/editor/converters/converterbase.h"
#include "runtime/editor/components/component_console.h"
#include "runtime/editor/components/component_button.h"
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
		, m_HasCompletedConversionProcess(false)
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

		// Reset conversion completed
		m_HasCompletedConversionProcess = false;

		// Add status bar
		m_StatusBar = std::make_unique<StatusBar>(m_MainTextField);
		m_StatusBar->SetText(m_Converter->GetName());

		// Activate converter
		m_Converter->Activate(m_Data, m_DataSize, m_Platform, m_MainTextField, m_ComponentsManager.get());

		// Add exit button
		auto button_cancel = std::make_shared<ComponentButton>(0x80, 0, nullptr,
			m_MainTextField, "Cancel",
			m_MainTextField->GetDimensions().m_Width - 11, m_MainTextField->GetDimensions().m_Height - 2,
			10,
			[&]() { m_ExitScreenCallback(); });
		m_ComponentsManager->AddComponent(button_cancel);

		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		m_ComponentsManager->SetGroupEnabledForTabbing(0);

		m_ComponentsManager->SetComponentInFocus(button_cancel);
	}


	void ScreenConvert::Deactivate()
	{
		m_ComponentsManager->Clear();

		m_Converter = nullptr;
		m_StatusBar = nullptr;

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
		m_Converter->ConsumeKeyEvent(inKeyEvent, inModifiers);

		if (inKeyEvent == SDLK_ESCAPE)
		{
			m_ExitScreenCallback();
			return true;
		}

		return false;
	}


	void ScreenConvert::Update(int inDeltaTick)
	{
		ScreenBase::Update(inDeltaTick);

		if (m_StatusBar != nullptr)
			m_StatusBar->Update(inDeltaTick);

		ComponentConsole& cout = *m_Console;

		if (m_Converter != nullptr)
			m_Converter->Update();
		if (m_Converter->GetState() == ConverterBase::State::Completed)
		{
			if (!m_HasCompletedConversionProcess)
			{
				m_HasCompletedConversionProcess = true;

				if (m_Converter->GetResult() != nullptr)
				{
					auto button_ok = std::make_shared<ComponentButton>(0x81, 0, nullptr,
						m_MainTextField, "Ok",
						m_MainTextField->GetDimensions().m_Width - 22, m_MainTextField->GetDimensions().m_Height - 2,
						10,
						[&]()
					{
						m_SuccessfullConversionCallback(this, m_PathAndFilename, m_Converter->GetResult());
						m_ExitScreenCallback();
					});

					m_ComponentsManager->AddComponent(button_ok);
					m_ComponentsManager->SetComponentInFocus(button_ok);
				}
			}
		}
	}
}

