#pragma once

#include "screen_base.h"

#include "runtime/editor/edit_state.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_state.h"
#include "runtime/editor/undo/undo.h"
#include "runtime/editor/dialog/dialog_selection_list.h"
#include "runtime/editor/dialog/dialog_move_selection_list.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_songs.h"

#include <memory>
#include <vector>
#include <functional>

namespace Emulation
{
	class CPUMemory;
	class ExecutionHandler;
	class SIDProxy;
}

namespace Utility
{
	template<typename CONTEXT>
	class KeyHook;
}

namespace Editor
{
	class ComponentsManager;

	class DataSourceTrackComponents;
	class DataSourceOrderList;
	class DataSourceSequence;
	class DataSourceTable;

	class ComponentTrack;
	class ComponentTracks;
	class ComponentOrderListOverview;
	class ComponentTableRowElements;
	class ComponentStringListSelector;

	class OverlayFlightRecorder;

	class DebugViews;

	class ScreenEdit final : public ScreenBase
	{
		struct DynamicKeysContext
		{
			SDL_Keycode m_KeyCode;
			ComponentsManager& m_ComponentsManager;
		};

		struct KeyTableIDPair
		{
			SDL_Keycode m_AssociatedKeyCode;
			int m_LetterIndex;
			int m_TableID;
		};

	public:
		static const unsigned char OrderListOverviewID;
		static const unsigned char PlayMarkerListID;
		static const unsigned char TracksTableID;
		static const unsigned char PulseFilterVisualizerID;

		ScreenEdit(
			Foundation::Viewport* inViewport, 
			Foundation::TextField* inMainTextField,
			CursorControl* inCursorControl, 
			DisplayState& inDisplayState,
			Utility::KeyHookStore& inKeyHookStore,
			EditState& inEditState, 
			Emulation::CPUMemory* inCPUMemory, 
			Emulation::ExecutionHandler* inExecutionHandler, 
			Emulation::SIDProxy* inSIDProxy,
			std::shared_ptr<DriverInfo>& inDriverInfo,
			std::function<void(void)> inRequestLoadCallback,
			std::function<void(void)> inRequestSaveCallback,
			std::function<void(void)> inRequestImportCallback,
			std::function<void(void)> inRequestLoadInstrumentCallback,
			std::function<void(void)> inRequestSaveInstrumentCallback,
			std::function<void(void)> inQuickSaveCallback,
			std::function<void(unsigned short, unsigned char)> inPackCallback,
			std::function<void(void)> inToggleShowOverlay,
			std::function<void(unsigned int)> inConfigReload);
		virtual ~ScreenEdit();

		void SetAdditionalConfiguration(bool inConvertLegacyDriverTableDefaultColors);

		void Activate() override;
		void Deactivate() override;

		void TryQuit(std::function<void(bool)> inResponseCallback) override;
		void TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;
		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;
		void Update(int inDeltaTick) override;
		void Refresh() override;

		void SetActivationMessage(const std::string& inMessage);
		void SetActivationTableFocusID(int inFocusComponentID, int inSelectedRow);
		void SetStatusBarMessage(const std::string& inMessage, int inDisplayDuration);
		void FlushUndo();

	private:
		bool ConsumeInputNotePlay(const Foundation::Keyboard& inKeyboard);

		void DoPlay();
		void DoPlay(unsigned int inEventPos);
		void DoPlayFromSelectedMarker();
		void DoStop();
		void DoToggleMute(unsigned int inChannel);
		void DoClearAllMuteState();
		void DoRestoreMuteState();
		void DoMoveToEventPositionOfSelectedMarker();

		bool IsPlaying() const;

		void DoSpaceBarFromTable(bool inPressed, bool inForceApplyCommand);

		void DoIncrementHighlightIntervalOrOffset(bool inOffset);
		void DoDecrementHighlightIntervalOrOffset(bool inOffset);

		void DoToggleSharpFlat();
		void DoOctaveChange(bool inUp);
		void DoToggleSIDModelAndRegion(bool inToggleRegion);
		void DoToggleContextHighlight();
		void DoToggleFollowPlay();
		void DoIncrementInstrumentIndex();
		void DoDecrementInstrumentIndex();
		void DoIncrementCommandIndex();
		void DoDecrementCommandIndex();
		void DoSetOrderlistLoopPointAll();

		void DoUtilitiesDialog();
		void DoOptionsDialog();
		void DoSongsDialog();

		void DoLoadSong();
		void DoLoadInstrument();
		void DoLoadImportSong();
		void DoSaveSong();
		void DoSaveInstrument();

		void PrepareMusicData();
		void PrepareLayout();

		void ExecuteTableInsertDeleteRule(int inTableID, int inIndexPre, int inIndexPost);
		void ExecuteTableInsertDeleteRules(const DriverInfo::TableInsertDeleteRules& inTableRules, int inSourceTableID, int inIndexPre, int inIndexPost);
		void ExecuteTableAction(int inTableID, int inActionInput);

		void OnDriverPostInitPlayFromEventPos(Emulation::CPUMemory* inCPUMemory, int inEventPosition);
		void OnDriverPostApplyChannelMuteState(Emulation::CPUMemory* inCPUMemory, int inTrack);
		void OnDriverPostApplyAllChannelsMuteState(Emulation::CPUMemory* inCPUMemory);
		void OnDriverPostClearMuteState(Emulation::CPUMemory* inCPUMemory);
		void OnDriverPostUpdate(Emulation::CPUMemory* inCPUMemory);

		void SetStatusPlaying(bool inIsPlaying);
		void SetStatusPlayingInput();

		void RestoreSIDOffsetData();

		void ConfigureKeyHooks();
		void ConfigureDynamicKeyHooks();
		void ConfigureNoteKeys();
		void ConfigurePlaybackOptions();

		void ShowSequenceUsageCount(unsigned char inSequenceIndex);

		template<typename EXECUTION_CALLBACK>
		void StartSongsDialogWithSelectionExecution(const std::string& headline, EXECUTION_CALLBACK&& inExecutionCallback);
		template<typename EXECUTION_CALLBACK>
		void StartMoveSongDialogWithSelectionExecution(const std::string& inCaption, EXECUTION_CALLBACK&& inExecutionCallback);
		
		// Load/save requests
		std::function<void(void)> m_LoadRequestCallback;
		std::function<void(void)> m_SaveRequestCallback;
		std::function<void(void)> m_ImportRequestCallback;
		std::function<void(void)> m_LoadInstrumentRequestCallback;
		std::function<void(void)> m_SaveInstrumentRequestCallback;
		std::function<void(void)> m_QuickSaveCallback;
		std::function<void(unsigned short, unsigned char)> m_PackCallback;
		std::function<void(void)> m_ToggleShowOverlay;
		std::function<void(unsigned int)> m_ConfigReconfigure;

		// Dynamic key codes
		std::vector<Utility::KeyHook<bool(DynamicKeysContext&)>> m_DynamicKeyHooks;
		std::vector<KeyTableIDPair> m_KeyTableIDPairs;

		// Status bar
		std::string m_DriverInfoString;
		std::string m_ActivationMessage;

		// Editor options
		EditState& m_EditState;

		// Driver state
		DriverState m_DriverState;

		// Fast forward
		std::vector<Utility::KeyHook<bool(void)>> m_FastForwardKeyHooks;
		unsigned int m_FastForwardFactor;

		// Timer
		int m_PlayTimerTicks;
		int m_PlayTimerSeconds;

		// Play note
		int m_LastPlayNote;
		int m_LastPlayNoteKeyInput;

		// Emulation domain
		std::shared_ptr<DriverInfo>& m_DriverInfo;

		Emulation::SIDProxy* m_SIDProxy;
		Emulation::CPUMemory* m_CPUMemory;
		Emulation::ExecutionHandler* m_ExecutionHandler;

		// Data sources
		std::vector<std::shared_ptr<DataSourceOrderList>> m_OrderListDataSources;
		std::vector<std::shared_ptr<DataSourceOrderList>> m_NotSelectedSongOrderListDataSources;
		std::vector<std::shared_ptr<DataSourceSequence>> m_SequenceDataSources;
		std::shared_ptr<DataSourceTable> m_InstrumentTableDataSource;
		std::shared_ptr<DataSourceTable> m_CommandTableDataSource;
		std::shared_ptr<DataSourceTrackComponents> m_TracksDataSource;

		// Components
		std::shared_ptr<ComponentTracks> m_TracksComponent;
		std::shared_ptr<ComponentOrderListOverview> m_OrderListOverviewComponent;
		std::shared_ptr<ComponentStringListSelector> m_PlayMarkerListComponent;
		std::shared_ptr<ComponentTableRowElements> m_InstrumentTableComponent;
		std::shared_ptr<ComponentTableRowElements> m_CommandTableComponent;

		bool m_ActivationFocusOnComponent;
		int m_ActivationComponentFocusID;
		int m_ActivationSelectedRow;

		// Overlay
		std::shared_ptr<OverlayFlightRecorder> m_OverlayFlightRecorder;

		// Undo
		std::shared_ptr<Undo> m_Undo;

		// Track data status report variables
		bool m_IsTrackDataReportSequence;
		int m_CurrentTrackDataIndex;
		int m_CurrentTrackDataPackedSize;

		// Playback 
		int m_LastPlaybackStartEventPos;
		int m_PlaybackCurrentEventPos;
		bool m_StopEmulationIfDriverStops;

		// Added configuration
		bool m_ConvertLegacyDriverTableDefaultColors;

		// Packing
		unsigned short m_PackingDestinationAddress;

		// Debug
		std::unique_ptr<DebugViews> m_DebugViews;
	};


	template<typename EXECUTION_CALLBACK>
	void ScreenEdit::StartSongsDialogWithSelectionExecution(const std::string& inCaption, EXECUTION_CALLBACK&& inExecutionCallback)
	{
		std::vector<std::string> selections;

		const unsigned char song_count = m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSongCount();
		const unsigned char selected_song = m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSelectedSong();

		for (unsigned char i = 0; i < song_count; ++i)
		{
			unsigned char song_num = i + 1;
			std::string selection_string = "Song " + std::to_string(song_num) + (song_num < 10 ? "  [" : " [") + m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSongName(i) + "]";
			selections.push_back(selection_string);
		}

		m_ComponentsManager->StartDialog(
			std::make_shared<DialogSelectionList>
			(
				60,
				song_count + 3,
				selected_song,
				inCaption,
				selections,
				inExecutionCallback,
				[]() {}
			)
		);
	}


	template<typename EXECUTION_CALLBACK>
	void ScreenEdit::StartMoveSongDialogWithSelectionExecution(const std::string& inCaption, EXECUTION_CALLBACK&& inExecutionCallback)
	{
		std::vector<std::string> selections;

		const unsigned char song_count = m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSongCount();
		const unsigned char selected_song = m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSelectedSong();

		for (unsigned char i = 0; i < song_count; ++i)
		{
			unsigned char song_num = i + 1;
			std::string selection_string = "Song " + std::to_string(song_num) + (song_num < 10 ? "  [" : " [") + m_DriverInfo->GetAuxilaryDataCollection().GetSongs().GetSongName(i) + "]";
			selections.push_back(selection_string);
		}

		m_ComponentsManager->StartDialog(
			std::make_shared<DialogMoveSelectionList>
			(
				60,
				song_count + 3,
				selected_song,
				inCaption,
				selections,
				inExecutionCallback,
				[]() {}
				)
		);
	}
}
