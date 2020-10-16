#pragma once

#include "screen_base.h"
#include "runtime/editor/components/component_text_input.h"
#include "runtime/editor/datasources/datasource_memory_buffer_string.h"

#include <string>
#include <vector>
#include <functional>

namespace Utility
{
	class ConfigFile;
}

namespace Foundation
{
	class IPlatform;
}

namespace Editor
{
	class ComponentsManager;
	class DataSourceDirectory;
	enum FileType : int;

	class ScreenDisk final : public ScreenBase
	{
	public:
		enum Mode : int
		{
			Load,
			Save,
			Import,
			LoadInstrument,
			SaveInstrument,
			SavePacked
		};

		ScreenDisk(
			Foundation::IPlatform* inPlatform, 
			Foundation::Viewport* inViewport, 
			Foundation::TextField* inMainTextField, 
			CursorControl* inCursorControl,
			DisplayState& inDisplayState,
			Utility::KeyHookStore& inKeyHookStore,
			Utility::ConfigFile& inConfigFile,
			std::function<void(const std::string&, FileType)> inSelectionCallback,
			std::function<void(void)> inCancelCallback);
		virtual ~ScreenDisk();

		void Activate() override;
		void Deactivate() override;

		void TryQuit(std::function<void(bool)> inResponseCallback) override;
		void TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback) override;

		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;
		void Update(int inDeltaTick) override;
		void Refresh() override;

		void SetMode(Mode inMode);
		const Mode GetMode() const;

		void SetSuggestedFileName(const std::string& inSuggestedFileName);

	private:
		void PrepareLayout();

		Mode m_Mode;
		std::string m_SuggestedFileName;

		std::shared_ptr<DataSourceDirectory> m_DataSourceDirectory;
		std::shared_ptr<DataSourceMemoryBufferString> m_DataSourceFileNameBuffer;
		std::shared_ptr<ComponentTextInput> m_ComponentFileNameInput;

		Foundation::IPlatform* m_Platform;
		const Utility::ConfigFile& m_ConfigFile;

		std::function<void(const std::string&, FileType)> m_SelectionCallback;
		std::function<void(void)> m_CancelCallback;
	};
}
