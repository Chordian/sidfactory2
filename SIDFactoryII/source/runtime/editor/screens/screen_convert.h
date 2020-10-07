#pragma once

#include "screen_base.h"
#include "foundation/platform/iplatform.h"
#include "foundation/graphics/image.h"
#include "SDL.h"

#include <functional>
#include <memory>
#include <string>

namespace Utility
{
	class C64File;
}

namespace Editor
{
	class ConverterBase;
	class ComponentTextBox;

	class ScreenConvert final : public ScreenBase
	{
	public:
		ScreenConvert(
			Foundation::Viewport* inViewport,
			Foundation::TextField* inMainTextField,
			CursorControl* inCursorControl,
			DisplayState& inDisplayState,
			Utility::KeyHookStore& inKeyHookStore,
			Foundation::IPlatform* inPlatform,
			std::function<void(void)> inExitScreenCallback,
			std::function<bool(ScreenBase*, const std::string&, std::shared_ptr<Utility::C64File>)> inSuccessfullConversionCallback
		);

		void PassConverterAndData(
			const std::string& inPathAndFilename,
			std::shared_ptr<ConverterBase> inConverter, 
			void* inData, 
			unsigned int inDataSize
		);

		void Activate() override;
		void Deactivate() override;

		void TryQuit(std::function<void(bool)> inResponseCallback) override;
		void TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback) override;

		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;
		void Update(int inDeltaTick) override;
	
	private:
		Foundation::IPlatform* m_Platform;

		std::function<void(void)> m_ExitScreenCallback;
		std::function<bool(ScreenBase*, const std::string&, std::shared_ptr<Utility::C64File>)> m_SuccessfullConversionCallback;

		std::string m_PathAndFilename;
		std::shared_ptr<ConverterBase> m_Converter;
		std::shared_ptr<ComponentTextBox> m_Console;
		
		void* m_Data;
		unsigned int m_DataSize;
	};
}
