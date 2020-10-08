#include "screen_convert.h"

#include "runtime/editor/converters/converterbase.h"
#include "runtime/editor/components/component_console.h"
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
		m_Converter->Activate(m_Data, m_DataSize, m_Platform, m_MainTextField, m_ComponentsManager.get());
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

		ComponentConsole& cout = *m_Console;

		if (m_Converter != nullptr)
			m_Converter->Update();
	}
}

