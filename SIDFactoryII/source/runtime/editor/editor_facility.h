#pragma once

#include "runtime/editor/cursor_control.h"
#include "runtime/editor/display_state.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/edit_state.h"
#include "runtime/editor/keys/keyhook_setup.h"
#include "runtime/editor/overlay_control.h"
#include "utils/keyhookstore.h"
#include <functional>
#include <memory>
#include <string>

namespace Foundation
{
	class IPlatform;
	class Viewport;
	class TextField;
	class Keyboard;
	class Mouse;
	class AudioStream;
}

namespace Emulation
{
	class CPUmos6510;
	class CPUMemory;
	class SIDProxy;
	class ExecutionHandler;
	class FlightRecorder;
}

namespace Utility
{
	class ConfigFile;
}

namespace Editor
{
	class ScreenBase;
	class ScreenIntro;
	class ScreenEdit;
	class ScreenDisk;
	class ScreenConvert;
	class ConverterBase;

	enum FileType : int;

	class EditorFacility
	{
	public:
		static const unsigned int DefaultDialogWidth;

		EditorFacility(Foundation::Viewport* inViewport);
		~EditorFacility();

		void Start(const char* inFileToLoad);
		void Stop();

		void Update(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse, int inDeltaTicks);
		bool IsDone() const;

		void TryQuit();
		void TryLoad(const std::string inPathAndFilename);

		void OnWindowResized();

	  private:
		void Reconfigure(unsigned int inReconfigureOption);
		void UpdateOverlayEnableDisable();
		void ToggleFullScreen();

		void RequestScreen(ScreenBase* inRequestedScreen);
		void ForceRequestScreen(ScreenBase* inRequestedScreen);
		void SetCurrentScreen(ScreenBase* inCurrentScreen);
		void HandleScreenState();

		bool IsFileSF2(const std::string& inPathAndFilename);
		bool LoadFile(const std::string& inPathAndFilename);
		bool LoadFileForImport(const std::string& inPathAndFilename, std::shared_ptr<DriverInfo>& outDriverInfo, std::shared_ptr<Utility::C64File>& outC64File);
		bool LoadAndConvertFile(const std::string& inPathAndFilename, ScreenBase* inCallerScreen, std::function<void()> inSuccesfullConversionAction);
		bool SaveFile(const std::string& inSavename);
		bool SavePackedFile(const std::string& inSavename);
		bool SavePackedFileToSID(ScreenBase* inCallerScreen, const std::string& inSavename);

		void OnCancelScreen(ScreenBase* inCallerScreen);
		void OnFilenameSelection(ScreenBase* inCallerScreen, const std::string& inSelectedFilename, FileType inSaveFileType);
		void OnExitIntroScreen();
		void OnExitIntroScreenToLoad();
		bool OnConversionSuccess(ScreenBase* inCallerScreen, const std::string& inPathAndFilename, std::shared_ptr<Utility::C64File> inConversionResult);

		void OnPack(ScreenBase* inCallerScreen, unsigned short inDestinationAddress, unsigned char inFirstZeroPage);
		void OnQuickSave(ScreenBase* inCallerScreen);
		void OnSaveError(ScreenBase* inCallerScreen);

		void DoLoad(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);
		void DoSave(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);
		void DoLoadInstrument(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);
		void DoSaveInstrument(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);
		void DoQuickSave(ScreenBase* inCallerScreen);
		void DoImport(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);
		void DoSavePacked(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);
		void DoSavePackedToSID(ScreenBase* inCallerScreen, const std::string& inSelectedFilename);

		void SetLastSavedPathAndFilename(const std::string& inLastSavedPathAndFilename);
		std::string ConfigureColorsFromScheme(int inSchemeIndex, Foundation::Viewport& inViewport);

		std::vector<std::shared_ptr<ConverterBase>> GetConverters() const;

		bool m_IsDone;
		bool m_FlipOverlayState;
		bool m_IsFullScreen;

		int m_ColorSchemeCount;
		int m_SelectedColorScheme;

		Foundation::Viewport* m_Viewport;
		Foundation::TextField* m_TextField;
		Foundation::AudioStream* m_AudioStream;

		Emulation::CPUmos6510* m_CPU;
		Emulation::CPUMemory* m_CPUMemory;
		Emulation::SIDProxy* m_SIDProxy;
		Emulation::ExecutionHandler* m_ExecutionHandler;
		Emulation::FlightRecorder* m_FlightRecorder;

		EditState m_EditState;
		DisplayState m_DisplayState;

		CursorControl m_CursorControl;

		KeyHookSetup m_KeyHookSetup;

		ScreenBase* m_RequestedScreen;
		ScreenBase* m_CurrentScreen;

		std::shared_ptr<DriverInfo> m_DriverInfo;
		std::unique_ptr<OverlayControl> m_OverlayControl;

		std::string m_LastSF2PathAndFilename;

		std::unique_ptr<ScreenIntro> m_IntroScreen;
		std::unique_ptr<ScreenEdit> m_EditScreen;
		std::unique_ptr<ScreenDisk> m_DiskScreen;
		std::unique_ptr<ScreenConvert> m_ConvertScreen;

		std::shared_ptr<Utility::C64File> m_PackedData;
	};
}
