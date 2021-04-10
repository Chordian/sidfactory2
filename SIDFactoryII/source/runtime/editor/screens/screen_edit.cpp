#include "screen_edit.h"
#include "screen_edit_utils.h"

#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"

#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_hardware_preferences.h"
#include "runtime/editor/auxilarydata/auxilary_data_play_markers.h"
#include "runtime/editor/utilities/editor_utils.h"
#include "runtime/editor/utilities/datasource_utils.h"
#include "runtime/editor/driver/driver_info.h"
#include "runtime/editor/driver/driver_utils.h"
#include "runtime/editor/driver/idriver_architecture.h"
#include "runtime/editor/components/component_track.h"
#include "runtime/editor/components/component_tracks.h"
#include "runtime/editor/components/component_table_row_elements.h"
#include "runtime/editor/components/component_table_row_elements_with_text.h"
#include "runtime/editor/components/component_orderlistoverview.h"
#include "runtime/editor/components/component_string_list_selector.h"
#include "runtime/editor/datasources/datasource_track_components.h"
#include "runtime/editor/datasources/datasource_table_column_major.h"
#include "runtime/editor/datasources/datasource_table_row_major.h"
#include "runtime/editor/datasources/datasource_play_markers.h"
#include "runtime/editor/datasources/datasource_flightrecorder.h"
#include "runtime/editor/datasources/datasource_sequence.h"
#include "runtime/editor/datasources/datasource_table_text.h"
#include "runtime/editor/visualizer_components/vizualizer_component_emulation_state.h"
#include "runtime/editor/debug/debug_views.h"
#include "runtime/editor/dialog/dialog_utilities.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/dialog/dialog_message_yesno.h"
#include "runtime/editor/dialog/dialog_hex_value_input.h"
#include "runtime/editor/dialog/dialog_optimize.h"
#include "runtime/editor/screens/statusbar/status_bar_edit.h"
#include "runtime/editor/overlays/overlay_flightrecorder.h"
#include "runtime/emulation/cpumemory.h"
#include "runtime/emulation/sid/sidproxy.h"
#include "runtime/emulation/sid/sidproxydefines.h"
#include "runtime/execution/executionhandler.h"

#include "utils/delegate.h"
#include "utils/keyhook.h"
#include "utils/keyhookstore.h"
#include "utils/usercolors.h"

#include "SDL.h"
#include <cctype>
#include "foundation/base/assert.h"
#include <algorithm>


using namespace Foundation;
using namespace Emulation;
using namespace Utility;

namespace Editor
{
	const unsigned char ScreenEdit::OrderListOverviewID = 0x40;
	const unsigned char ScreenEdit::PlayMarkerListID = 41;
	const unsigned char ScreenEdit::TracksTableID = 0x42;

	ScreenEdit::ScreenEdit(
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
		std::function<void(unsigned short)> inPackCallback,
		std::function<void(void)> inToggleShowOverlay,
		std::function<void(unsigned int)> inReconfigure)
		: ScreenBase(inViewport, inMainTextField, inCursorControl, inDisplayState, inKeyHookStore)
		, m_EditState(inEditState)
		, m_CPUMemory(inCPUMemory)
		, m_ExecutionHandler(inExecutionHandler)
		, m_SIDProxy(inSIDProxy)
		, m_DriverInfo(inDriverInfo)
		, m_IsTrackDataReportSequence(false)
		, m_CurrentTrackDataIndex(0)
		, m_CurrentTrackDataPackedSize(0)
		, m_PlaybackCurrentEventPos(-1)
		, m_LoadRequestCallback(inRequestLoadCallback)
		, m_SaveRequestCallback(inRequestSaveCallback)
		, m_ImportRequestCallback(inRequestImportCallback)
		, m_LoadInstrumentRequestCallback(inRequestLoadInstrumentCallback)
		, m_SaveInstrumentRequestCallback(inRequestSaveInstrumentCallback)
		, m_QuickSaveCallback(inQuickSaveCallback)
		, m_PackCallback(inPackCallback)
		, m_ToggleShowOverlay(inToggleShowOverlay)
		, m_ConfigReconfigure(inReconfigure)
		, m_PlayTimerTicks(0)
		, m_PlayTimerSeconds(0)
		, m_LastPlayNote(0x30)
		, m_ActivationMessage("")
		, m_ConvertLegacyDriverTableDefaultColors(false)
	{
	}


	ScreenEdit::~ScreenEdit()
	{
	}


	//------------------------------------------------------------------------------------------------------------


	void ScreenEdit::SetAdditionalConfiguration(bool inConvertLegacyDriverTableDefaultColors)
	{
		m_ConvertLegacyDriverTableDefaultColors = inConvertLegacyDriverTableDefaultColors;
	}


	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::Activate()
	{
		ScreenBase::Activate();

		// Clear undo
		FlushUndo();

		// Prepare data (tracks and sequences)
		PrepareMusicData();

		// Clear the text field
		ClearTextField();

		// Configure keys
		ConfigureKeyHooks();
		ConfigureDynamicKeyHooks();

		// Prepare the layout
		PrepareLayout();

		// Enabled update of the driver in the emulation domain
		m_ExecutionHandler->SetEnableUpdate(true);
		m_ExecutionHandler->Lock();

		// Apply sid settings
		const auto& hardware_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetHardwarePreferences();

		const AuxilaryDataHardwarePreferences::SIDModel sid_model = hardware_preferences.GetSIDModel();
		m_SIDProxy->SetModel(sid_model == AuxilaryDataHardwarePreferences::SIDModel::MOS6581 ? SID_MODEL_6581 : SID_MODEL_8580);

		const AuxilaryDataHardwarePreferences::Region hardware_region = hardware_preferences.GetRegion();
		m_SIDProxy->SetEnvironment(hardware_region == AuxilaryDataHardwarePreferences::Region::PAL ? SID_ENVIRONMENT_PAL : SID_ENVIRONMENT_NTSC);
		m_SIDProxy->ApplySettings();

		m_ExecutionHandler->Unlock();

		// Create debug views
		m_DebugViews = std::make_unique<DebugViews>(m_Viewport, &*m_ComponentsManager, m_CPUMemory, m_MainTextField->GetDimensions(), m_DriverInfo);

		// Set SID factory text for a couple of seconds
		auto mouse_button_octave = [&](Foundation::Mouse::Button inMouseButton, int inKeyboardModifiers)
		{
			int direction = (inMouseButton == Foundation::Mouse::Button::Left) ^ KeyboardUtils::IsModifierExclusivelyDown(inKeyboardModifiers, Keyboard::Shift);
			DoOctaveChange(direction);
		};

		auto mouse_button_flat_sharp = [&](Foundation::Mouse::Button inMouseButton, int inKeyboardModifiers)
		{
			DoToggleSharpFlat();
		};

		auto mouse_button_sid_model = [&](Foundation::Mouse::Button inMouseButton, int inKeyboardModifiers)
		{
			DoToggleSIDModelAndRegion(KeyboardUtils::IsModifierExclusivelyDown(inKeyboardModifiers, Keyboard::Control));
		};

		auto mouse_button_context_highlight = [&](Foundation::Mouse::Button inMouseButton, int inKeyboardModifiers)
		{
			DoToggleContextHighlight();
		};

		auto mouse_button_follow_play = [&](Foundation::Mouse::Button inMouseButton, int inKeyboardModifiers)
		{
			DoToggleFollowPlay();
		};

		// Reset the driver status struct
		m_DriverState = DriverState();

		// Create the status bar
		m_StatusBar = std::make_unique<StatusBarEdit>(m_MainTextField, m_EditState, m_DriverState, m_DriverInfo->GetAuxilaryDataCollection(), mouse_button_octave, mouse_button_flat_sharp, mouse_button_sid_model, mouse_button_context_highlight, mouse_button_follow_play);
		m_StatusBar->SetText(m_ActivationMessage.length() > 0 ? m_ActivationMessage : " SID Factory II", 2500);
		m_ActivationMessage = "";

		// Create flight recorder overlay
		m_OverlayFlightRecorder = std::make_shared<OverlayFlightRecorder>(m_Viewport, &*m_ComponentsManager, m_CPUMemory, m_ExecutionHandler, m_MainTextField->GetDimensions());

		// Set post update callback from emulation context
		m_ExecutionHandler->SetPostUpdateCallback([&](CPUMemory* inCPUMemory) { OnDriverPostUpdate(inCPUMemory); });

		// Set driver data for flight recorder
		auto& driver_common = m_DriverInfo->GetDriverCommon();
		m_ExecutionHandler->GetFlightRecorder()->SetDriverSyncReadAddress(driver_common.m_TriggerSyncAddress);
		m_ExecutionHandler->GetFlightRecorder()->SetDriverTempoCounterReadAddress(driver_common.m_TempoCounterAddress);

		// Make sure the driver is stopped upon entering the screen
		m_ExecutionHandler->QueueStop();
		SetStatusPlaying(false);

		m_PlaybackCurrentEventPos = -1;

		// Reset edit state
		m_EditState.SetSelectedInstrument(static_cast<char>(m_InstrumentTableComponent->GetSelectedRow()));
	}


	void ScreenEdit::Deactivate()
	{
		// Dereference flight recorder overlay
		m_OverlayFlightRecorder = nullptr;

		// Disable flight recorder
		m_ExecutionHandler->GetFlightRecorder()->SetRecording(false);

		// Remove post update callback (dependencies are going to be removed after, so this will avoid tearing down the application)
		m_ExecutionHandler->SetPostUpdateCallback(nullptr);

		// Restore muted tracks
		RestoreSIDOffsetData();

		// Dereference the status bar
		m_StatusBar = nullptr;

		// Clear / dereference data sources
		m_OrderListDataSources.clear();
		m_SequenceDataSources.clear();
		m_InstrumentTableDataSource = nullptr;
		m_CommandTableDataSource = nullptr;
		m_TracksDataSource = nullptr;

		// Components
		m_TracksComponent = nullptr;

		// Dereference debug views
		m_DebugViews = nullptr;

		// Clear the components manager
		m_ComponentsManager->Clear();

		// Reset play event position
		m_PlaybackCurrentEventPos = -1;
	}


	void ScreenEdit::TryQuit(std::function<void(bool)> inResponseCallback)
	{
		DoStop();
		ShowQuitDialog(m_ComponentsManager, inResponseCallback);
	}


	void ScreenEdit::TryLoad(const std::string& inPathAndFilename, std::function<void(bool)> inResponseCallback)
	{
		DoStop();
		ShowTryLoadDialog(inPathAndFilename, m_ComponentsManager, inResponseCallback);
	}

	//------------------------------------------------------------------------------------------------------------

	bool ScreenEdit::ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse)
	{
		m_FastForwardFactor = 0;

		// Process but do not consume, Fast forward
		for (const auto key_event : inKeyboard.GetKeyDownList())
			Utility::ConsumeInputKeyHooks(key_event, inKeyboard.GetModiferMask(), m_FastForwardKeyHooks);

		// Apply fast forward factor to the execution handler
		m_ExecutionHandler->SetFastForward(m_FastForwardFactor);

		// Consume input for note play, if allowed
		if (ConsumeInputNotePlay(inKeyboard))
			return true;

		// Do the rest
		if (ScreenBase::ConsumeInput(inKeyboard, inMouse))
			return true;


		if (m_DebugViews != nullptr)
		{
			m_DebugViews->Update(inKeyboard, inMouse);
			m_DebugViews->SetEventPosition(m_PlaybackCurrentEventPos);
		}

		return false;
	}


	bool ScreenEdit::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		if(Utility::ConsumeInputKeyHooks(inKeyEvent, inModifiers, m_DynamicKeyHooks, DynamicKeysContext({ inKeyEvent, *m_ComponentsManager })))
			return true;

		if (KeyboardUtils::IsModifierExclusivelyDown(inModifiers, Keyboard::Alt))
		{
			if (inKeyEvent >= SDLK_1 && inKeyEvent <= SDLK_8)
			{
				int play_marker_selection = static_cast<int>(inKeyEvent) - static_cast<int>(SDLK_1);
				m_PlayMarkerListComponent->SetSelectionIndex(play_marker_selection);
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::Update(int inDeltaTick)
	{
		if (m_StatusBar != nullptr)
			m_StatusBar->Update(inDeltaTick);

		m_TracksComponent->TellPlaybackEventPosition(m_PlaybackCurrentEventPos);
		m_OrderListOverviewComponent->TellPlaybackEventPosition(m_PlaybackCurrentEventPos);

		m_ExecutionHandler->Lock();
		if (m_ExecutionHandler->IsInErrorState())
		{
			std::string error_message = m_ExecutionHandler->GetErrorMessage();
			DoStop();
			m_ExecutionHandler->ClearErrorState();

			m_ComponentsManager->StartDialog(std::make_shared<DialogMessage>("Emulation error!", error_message, 60, true, []() {}));
		}
		m_ExecutionHandler->Unlock();

		m_ComponentsManager->Update(inDeltaTick, m_CPUMemory);

		// Update play timer
		const bool is_playing = m_DriverState.GetPlayState() == Editor::DriverState::PlayState::Playing;
		if (is_playing)
		{
			m_PlayTimerTicks += inDeltaTick;

			while (m_PlayTimerTicks > 1000)
			{
				m_PlayTimerSeconds++;
				m_PlayTimerTicks -= 1000;
			}

			if (m_EditState.IsFollowPlayMode() && m_PlaybackCurrentEventPos >= 0)
				m_TracksComponent->SetEventPosition(m_PlaybackCurrentEventPos, true);
		}
	}


	void ScreenEdit::Refresh()
	{
		ScreenBase::Refresh();

		// Refresh info rect
		const int h = 4;
		const int x = m_TracksComponent->GetPosition().m_X + m_TracksComponent->GetDimensions().m_Width + 1;
		const int y = m_MainTextField->GetDimensions().m_Height - (h + 1);
		const int w = m_MainTextField->GetDimensions().m_Width - (x + 1);

		m_MainTextField->ColorAreaBackground(ToColor(UserColor::ScreenEditInfoRectBackground), x, y, w, h);

		const int minutes = m_PlayTimerSeconds / 60;
		const int seconds = m_PlayTimerSeconds % 60;

		m_MainTextField->Print(x + 1, y + 1, ToColor(IsPlaying() ? UserColor::ScreenEditInfoRectTextTimePlaybackState : UserColor::ScreenEditInfoRectText), "Playing time: " + std::to_string(minutes) + ((seconds < 10) ? ":0" : ":") + std::to_string(seconds) + "      ");
		m_MainTextField->Print(x + 1, y + 2, ToColor(UserColor::ScreenEditInfoRectText), m_DriverInfo->GetDescriptor().m_DriverName);

		// m_Undo->PrintDebug(*m_MainTextField);
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::SetActivationMessage(const std::string& inMessage)
	{
		m_ActivationMessage = " " + inMessage;
	}

	void ScreenEdit::SetStatusBarMessage(const std::string& inMessage, int inDisplayDuration)
	{
		if (m_StatusBar != nullptr)
			m_StatusBar->SetText(inMessage, inDisplayDuration);
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::FlushUndo()
	{
		m_Undo = std::make_shared<Undo>(*m_CPUMemory, *m_DriverInfo);
		m_Undo->SetOnRestoredStepComponentHandler([this](int inComponentID, int inComponentGroupID)
		{
			m_ComponentsManager->SetComponentInFocus(inComponentID);
		});
	}

	//------------------------------------------------------------------------------------------------------------

	bool ScreenEdit::ConsumeInputNotePlay(const Foundation::Keyboard& inKeyboard)
	{
		// Preview note input consumption before the rest
		const bool play_is_allowed = !m_ComponentsManager->IsNoteInputSilenced();
		const bool shift_exclusively_down = KeyboardUtils::IsModifierExclusivelyDown(inKeyboard.GetModiferMask(), Keyboard::Shift);
		const int current_octave = m_EditState.GetOctave();
		DriverState::PlayNote current_play_note = m_DriverState.GetPlayNote();

		if (play_is_allowed)
		{
			if (!m_ComponentsManager->IsComponentInFocus(m_TracksComponent->GetComponentID()) || m_DriverState.GetPlayState() != DriverState::PlayState::Playing)
			{
				if (shift_exclusively_down ||
					(inKeyboard.IsCapsLockDown() && m_DriverState.GetPlayState() == DriverState::PlayState::Playing_Input))
				{
					int new_note = -1;

					// Get key down events (just getting the last registered, since the driver playback is monophonic
					for (const auto& key_pressed : inKeyboard.GetKeyPressedList())
						new_note = EditorUtils::GetNoteValue(key_pressed, current_octave);

					if (new_note != -1)
					{
						current_play_note.m_Note = new_note;
						current_play_note.m_Instrument = m_InstrumentTableComponent->GetSelectedRow();
						current_play_note.m_Command = m_CommandTableComponent->GetSelectedRow();
						current_play_note.m_ApplyCommand = (m_CommandTableComponent->GetHasControl());

						m_DriverState.SetPlayNote(current_play_note);

						m_LastPlayNoteKeyInput = new_note;
					}
				}
			}

			// Get key release events
			for (const auto& key_released : inKeyboard.GetKeyReleasedList())
			{
				const int note_released = EditorUtils::GetNoteValue(key_released, current_octave);

				if (note_released == current_play_note.m_Note)
					m_DriverState.SetPlayNote(DriverState::PlayNote());
			}
		}
		else
		{
			if (current_play_note.m_Note != -1)
				m_DriverState.SetPlayNote(DriverState::PlayNote());
		}


		// Modify the play state
		if (current_play_note.m_Note != -1 && m_DriverState.GetPlayState() == DriverState::PlayState::Stopped)
			SetStatusPlayingInput();

		return current_play_note.m_Note != -1 && shift_exclusively_down;
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::DoPlay()
	{
		DoRestoreMuteState();

		// Push instruments data to emulation memory
		m_CPUMemory->Lock();
		m_InstrumentTableDataSource->PushDataToSource();
		m_CPUMemory->Unlock();

		m_ExecutionHandler->QueueInit(0);
		SetStatusPlaying(true);

		m_LastPlaybackStartEventPos = 0;
		m_PlaybackCurrentEventPos = -1;
	}


	void ScreenEdit::DoPlay(unsigned int inEventPos)
	{
		DoRestoreMuteState();

		// Push instruments data to emulation memory
		m_CPUMemory->Lock();
		m_InstrumentTableDataSource->PushDataToSource();
		m_CPUMemory->Unlock();

		m_ExecutionHandler->QueueInit(0, [&, inEventPos](Emulation::CPUMemory* inCPUMemory) { OnDriverPostInitPlayFromEventPos(inCPUMemory, inEventPos); });
		SetStatusPlaying(true);

		m_LastPlaybackStartEventPos = inEventPos;
		m_PlaybackCurrentEventPos = inEventPos - 1;
	}


	void ScreenEdit::DoPlayFromSelectedMarker()
	{
		// Get the event position at which to start playback from
		const int event_pos = [&]()
		{
			const int selected_marker = m_PlayMarkerListComponent->GetSelectionIndex();
			const auto& play_markers = m_DriverInfo->GetAuxilaryDataCollection().GetPlayMarkers();

			return play_markers.GetPlayMarkerEventPosition(selected_marker);
		}();

		DoPlay(event_pos);
	}


	void ScreenEdit::DoStop()
	{
		// Push instruments data to emulation memory
		m_CPUMemory->Lock();
		m_InstrumentTableDataSource->PushDataToSource();
		m_CPUMemory->Unlock();

		m_ExecutionHandler->QueueStop();
		SetStatusPlaying(false);
		DoClearAllMuteState();

		m_PlaybackCurrentEventPos = -1;
	}


	void ScreenEdit::DoToggleMute(unsigned int inChannel)
	{
		const bool muted = m_TracksComponent->IsMuted(inChannel);
		m_TracksComponent->SetMuted(inChannel, !muted);

		if (IsPlaying())
		{
			unsigned char channel = static_cast<unsigned char>(inChannel);
			m_ExecutionHandler->QueueMuteChannel(channel, [&, channel](Emulation::CPUMemory* inCPUMemory) { OnDriverPostApplyChannelMuteState(inCPUMemory, channel); });
		}
	}


	void ScreenEdit::DoClearAllMuteState()
	{
		m_ExecutionHandler->QueueClearAllMuteState([&](Emulation::CPUMemory* inCPUMemory) {OnDriverPostClearMuteState(inCPUMemory); });
	}


	void ScreenEdit::DoRestoreMuteState()
	{
		for(unsigned char i=0; i<m_DriverInfo->GetMusicData().m_TrackCount; ++i)
			m_ExecutionHandler->QueueMuteChannel(i, [&, i](Emulation::CPUMemory* inCPUMemory) { OnDriverPostApplyChannelMuteState(inCPUMemory, i); });
	}


	void ScreenEdit::DoMoveToEventPositionOfSelectedMarker()
	{
		const int selected_marker = m_PlayMarkerListComponent->GetSelectionIndex();
		auto& play_markers = m_DriverInfo->GetAuxilaryDataCollection().GetPlayMarkers();

		const int event_pos = play_markers.GetPlayMarkerEventPosition(selected_marker);

		m_TracksComponent->SetEventPosition(event_pos, false);
	}


	bool ScreenEdit::IsPlaying() const
	{
		return m_DriverState.GetPlayState() == Editor::DriverState::PlayState::Playing;
	}


	void ScreenEdit::DoSpaceBarFromTable(bool inPressed, bool inForceApplyCommand)
	{
		if (inPressed)
		{
			if ( m_DriverState.GetPlayState() == DriverState::PlayState::Playing_Input)
			{
				DriverState::PlayNote current_play_note = m_DriverState.GetPlayNote();

				current_play_note.m_Note = m_LastPlayNoteKeyInput;
				current_play_note.m_Instrument = m_InstrumentTableComponent->GetSelectedRow();
				current_play_note.m_Command = m_CommandTableComponent->GetSelectedRow();
				current_play_note.m_ApplyCommand = (m_CommandTableComponent->GetHasControl()) || inForceApplyCommand;

				m_DriverState.SetPlayNote(current_play_note);
			}
		}
		else
		{
			m_DriverState.SetPlayNote(DriverState::PlayNote());
		}
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::DoIncrementHighlightIntervalOrOffset(bool inOffset)
	{
		auto& editing_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetEditingPreferences();

		int offset = static_cast<int>(editing_preferences.GetEventPosHighlightOffset());
		int interval = static_cast<int>(editing_preferences.GetEventPosHighlightInterval());

		if (inOffset)
			offset++;
		else
			interval++;

		if (offset >= interval)
			offset = 0;

		editing_preferences.SetEventPosHighlightOffset(static_cast<unsigned char>(offset));
		editing_preferences.SetEventPosHighlightInterval(static_cast<unsigned char>(interval));

		m_TracksComponent->ForceRefresh();
	}


	void ScreenEdit::DoDecrementHighlightIntervalOrOffset(bool inOffset)
	{
		auto& editing_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetEditingPreferences();

		int offset = static_cast<int>(editing_preferences.GetEventPosHighlightOffset());
		int interval = static_cast<int>(editing_preferences.GetEventPosHighlightInterval());

		if (inOffset)
			offset--;
		else
		{
			if (interval > 1)
				interval--;
		}

		if (offset < 0)
			offset = interval - 1;

		editing_preferences.SetEventPosHighlightOffset(static_cast<unsigned char>(offset));
		editing_preferences.SetEventPosHighlightInterval(static_cast<unsigned char>(interval));

		m_TracksComponent->ForceRefresh();

	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::DoToggleSharpFlat()
	{
		auto& editing_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetEditingPreferences();

		AuxilaryDataEditingPreferences::NotationMode notation_mode = editing_preferences.GetNotationMode();
		editing_preferences.SetNotationMode(notation_mode == AuxilaryDataEditingPreferences::NotationMode::Sharp
			? AuxilaryDataEditingPreferences::NotationMode::Flat 
			: AuxilaryDataEditingPreferences::NotationMode::Sharp);

		m_TracksComponent->ForceRefresh();
	}

	void ScreenEdit::DoOctaveChange(bool inUp)
	{
		const unsigned int octave = m_EditState.GetOctave();

		if (inUp)
		{
			if (octave < 6)
				m_EditState.SetOctave(octave + 1);
		}
		else
		{
			if (octave > 0)
				m_EditState.SetOctave(octave - 1);
		}
	}

	void ScreenEdit::DoToggleSIDModelAndRegion(bool inToggleRegion)
	{
		auto& hardware_preferences = m_DriverInfo->GetAuxilaryDataCollection().GetHardwarePreferences();

		if (!inToggleRegion)
		{
			const AuxilaryDataHardwarePreferences::SIDModel sid_model = hardware_preferences.GetSIDModel() == AuxilaryDataHardwarePreferences::SIDModel::MOS6581 
				? AuxilaryDataHardwarePreferences::SIDModel::MOS8580 
				: AuxilaryDataHardwarePreferences::SIDModel::MOS6581;

			hardware_preferences.SetSIDModel(sid_model);

			m_ExecutionHandler->Lock();

			m_SIDProxy->SetModel(sid_model == AuxilaryDataHardwarePreferences::SIDModel::MOS6581 ? SID_MODEL_6581 : SID_MODEL_8580);
			m_SIDProxy->ApplySettings();

			m_ExecutionHandler->Unlock();
		}
		else
		{
			const AuxilaryDataHardwarePreferences::Region hardware_region = hardware_preferences.GetRegion() == AuxilaryDataHardwarePreferences::Region::PAL 
				? AuxilaryDataHardwarePreferences::Region::NTSC 
				: AuxilaryDataHardwarePreferences::Region::PAL;

			hardware_preferences.SetRegion(hardware_region);

			m_ExecutionHandler->Lock();

			m_SIDProxy->SetEnvironment(hardware_region == AuxilaryDataHardwarePreferences::Region::PAL ? SID_ENVIRONMENT_PAL : SID_ENVIRONMENT_NTSC);
			m_SIDProxy->ApplySettings();

			m_ExecutionHandler->Unlock();
		}
	}

	void ScreenEdit::DoToggleContextHighlight()
	{
		m_EditState.SetSequenceHighlighting(!m_EditState.IsSequenceHighlightingEnabled());
		m_TracksComponent->ForceRefresh();
	}


	void ScreenEdit::DoToggleFollowPlay()
	{
		bool follow_play_cursor = !m_EditState.IsFollowPlayMode();
		m_EditState.SetFollowPlayMode(follow_play_cursor);

		if (IsPlaying())
		{
			m_EditState.SetPreventSequenceEdit(follow_play_cursor);

			if (follow_play_cursor)
				m_TracksComponent->CancelFocusModeOrderList();
		}
	}


	void ScreenEdit::DoIncrementInstrumentIndex()
	{
		if (m_InstrumentTableComponent != nullptr)
			m_InstrumentTableComponent->DoCursorDown();
	}

	void ScreenEdit::DoDecrementInstrumentIndex()
	{
		if (m_InstrumentTableComponent != nullptr)
			m_InstrumentTableComponent->DoCursorUp();
	}

	void ScreenEdit::DoIncrementCommandIndex()
	{
		if (m_CommandTableComponent != nullptr)
			m_CommandTableComponent->DoCursorDown();
	}

	void ScreenEdit::DoDecrementCommandIndex()
	{
		if (m_CommandTableComponent != nullptr)
			m_CommandTableComponent->DoCursorUp();
	}

	void ScreenEdit::DoSetOrderlistLoopPointAll()
	{
		if (m_TracksComponent != nullptr)
		{
			m_TracksComponent->SetOrderlistLoopPointAll();
			m_OrderListOverviewComponent->ForceRefresh();
		}
	}



	void ScreenEdit::DoUtilitiesDialog()
	{
		if (!m_ComponentsManager->IsDisplayingDialog())
		{
			auto on_select = [&](const DialogUtilities::Selection inSelection)
			{
				switch (inSelection)
				{
				case DialogUtilities::Selection::Statistics:
					{
						m_CPUMemory->Lock();
						const bool is_uppercase = m_DisplayState.IsHexUppercase();

						std::string text = "Driver: " + m_DriverInfo->GetDescriptor().m_DriverName + "\n";
						text += "Highest sequence  : 0x" + EditorUtils::ConvertToHexValue(DriverUtils::GetHighestSequenceIndexUsed(*m_DriverInfo, *m_CPUMemory), is_uppercase) + "\n";
						text += "Highest instrument: 0x" + EditorUtils::ConvertToHexValue(DriverUtils::GetHighestInstrumentIndexUsed(*m_DriverInfo, *m_CPUMemory), is_uppercase) + "\n";
						text += "Highest command   : 0x" + EditorUtils::ConvertToHexValue(DriverUtils::GetHighestCommandIndexUsed(*m_DriverInfo, *m_CPUMemory), is_uppercase) + "\n";

						for (const auto& table_definition : m_DriverInfo->GetTableDefinitions())
						{
							const unsigned char highest_used_index = DriverUtils::GetHighestTableRowUsedIndex(table_definition, *m_CPUMemory);
							text += "Highest used index of table '" + table_definition.m_Name + "': 0x" + EditorUtils::ConvertToHexValue(highest_used_index, is_uppercase) + "\n";
						}

						m_CPUMemory->Unlock();

						m_ComponentsManager->StartDialog(std::make_shared<DialogMessage>("Statistics", text, 60, false, []() {}));						
					}
					break;
				case DialogUtilities::Selection::Optimize:
					m_ComponentsManager->StartDialog(std::make_shared<DialogOptimize>(
						m_OrderListDataSources,
						m_SequenceDataSources,
						m_InstrumentTableDataSource,
						m_CommandTableDataSource,
						m_InstrumentTableComponent->GetComponentID(),
						m_CommandTableComponent->GetComponentID(),
						*m_ComponentsManager,
						*m_DriverInfo,
						m_CPUMemory,
						[&]() 
						{
							m_InstrumentTableComponent->PullDataFromSource(false);
							m_CommandTableComponent->PullDataFromSource(false);

							m_ComponentsManager->ForceRefresh(); 
						}
					));
					break;
				case DialogUtilities::Selection::Pack:
					{
						auto dialog_ok = [this](unsigned int inDestinationAddress)
						{
							m_PackCallback(static_cast<unsigned short>(inDestinationAddress));
						};
						
						auto dialog_cancel = [this]()
						{
						};

						const unsigned int default_destination_address = 0x1000;
						m_ComponentsManager->StartDialog(std::make_shared<DialogHexValueInput>("Packer", "Packed song destination address:", 32, 4, default_destination_address, 0xffff, dialog_ok, dialog_cancel));
					}

					break;
				case DialogUtilities::Selection::ClearSequences:
					{
						auto dialog_yes = [&]()
						{
							m_CPUMemory->Lock();

							DataSourceUtils::ClearSequences(m_SequenceDataSources);
							DataSourceUtils::ClearOrderlist(m_OrderListDataSources);

							m_CPUMemory->Unlock();

							m_ComponentsManager->ForceRefresh();
							m_Undo->Clear();
						};

						m_ComponentsManager->StartDialog(std::make_shared<DialogMessageYesNo>("Clear all sequences and orderlists!", "Are you sure you want to proceed? This cannot be undone!", 60, dialog_yes, []() {}));
					}
					break;
				case DialogUtilities::Selection::ExpandSequences:
					{
						auto dialog_yes = [&]()
						{
							m_CPUMemory->Lock();

							DataSourceUtils::ExpandSequences(m_SequenceDataSources);
		
							m_CPUMemory->Unlock();

							m_ComponentsManager->ForceRefresh();
							m_Undo->Clear();
						};

						m_ComponentsManager->StartDialog(std::make_shared<DialogMessageYesNo>("Expand all sequences and orderlists!", "Are you sure you want to proceed? This cannot be undone!", 60, dialog_yes, []() {}));
					}
					break;
				}
			};

			m_ComponentsManager->StartDialog(std::make_shared<DialogUtilities>(60, 8, on_select, [&]() { DoRestoreMuteState(); }));
		}
	}


	void ScreenEdit::DoOptionsDialog()
	{
		m_ComponentsManager->StartDialog(std::make_shared<DialogMessage>("Not implemented", "Options!", 60, true, []() {}));
	}


	void ScreenEdit::DoLoadSong()
	{
		m_LoadRequestCallback();
	}


	void ScreenEdit::DoLoadInstrument()
	{
		m_ComponentsManager->StartDialog(std::make_shared<DialogMessageYesNo>("Not implemented", "Load instrument!", 60, [&]() { m_LoadInstrumentRequestCallback(); }, []() {}));
	}


	void ScreenEdit::DoLoadImportSong()
	{
		m_ImportRequestCallback();
	}


	void ScreenEdit::DoSaveSong()
	{
		m_SaveRequestCallback();
	}


	void ScreenEdit::DoSaveInstrument()
	{
		m_ComponentsManager->StartDialog(std::make_shared<DialogMessageYesNo>("Not implemented", "Save instrument!", 60, [&]() { m_SaveInstrumentRequestCallback(); }, []() {}));
	}

	//------------------------------------------------------------------------------------------------------------

	void ScreenEdit::PrepareMusicData()
	{
		FOUNDATION_ASSERT(m_DriverInfo != nullptr);
		FOUNDATION_ASSERT(m_CPUMemory != nullptr);

		Undo* undo = &(*m_Undo);

		// Make sure there's valid displayable data in all sequences
		m_CPUMemory->Lock();
		ScreenEditUtils::PrepareSequenceData(*m_DriverInfo, *m_CPUMemory);
		ScreenEditUtils::PrepareSequencePointers(*m_DriverInfo, *m_CPUMemory);
		m_CPUMemory->Unlock();

		// Create data containers for each track
		ScreenEditUtils::PrepareOrderListsDataSources(*m_DriverInfo, *m_CPUMemory, m_OrderListDataSources);

		// Create data containers for each sequence
		ScreenEditUtils::PrepareSequenceDataSources(*m_DriverInfo, m_DriverState, *m_CPUMemory, m_SequenceDataSources);

		// Status report lamda for sequence editing
		auto sequence_editing_status_report = [&](bool inIsSequenceReport, int inDataIndex, int inPackedSize)
		{
			m_IsTrackDataReportSequence = inIsSequenceReport;
			m_CurrentTrackDataIndex = inDataIndex;
			m_CurrentTrackDataPackedSize = inPackedSize;
		};

		auto get_first_free_sequence_index = [&]() -> unsigned char
		{
			m_CPUMemory->Lock();
			unsigned char first_free_sequence_index = DriverUtils::GetFirstUnusedSequenceIndex(*m_DriverInfo, *m_CPUMemory);
			m_CPUMemory->Unlock();

			return first_free_sequence_index;
		};
        
        auto get_first_empty_sequence_index = [&]() -> unsigned char
        {
            m_CPUMemory->Lock();
            unsigned char first_free_sequence_index = DriverUtils::GetFirstEmptySequenceIndex(*m_DriverInfo, *m_CPUMemory);
            m_CPUMemory->Unlock();

            return first_free_sequence_index;
        };

		// Create copy/paste data container
		m_TrackCopyPasteData = std::make_shared<TrackCopyPasteData>();

		// Create data container for music data (which is all tracks and sequences combined)
		std::vector<std::shared_ptr<ComponentTrack>> tracks;

		for (auto& order_list_data_source : m_OrderListDataSources)
			tracks.push_back(
				std::make_shared<ComponentTrack>(
					-1,
					-1,
					undo,
					order_list_data_source,
					m_SequenceDataSources,
					m_MainTextField,
					m_EditState,
					m_KeyHookStore,
					m_DriverInfo->GetAuxilaryDataCollection(),
					m_TrackCopyPasteData,
					sequence_editing_status_report,
					get_first_free_sequence_index,
                    get_first_empty_sequence_index,
					0, 0, 0
					)
			);

		m_TracksDataSource = std::make_shared<DataSourceTrackComponents>(tracks);
	}

	void ScreenEdit::PrepareLayout()
	{
		Undo* undo = &(*m_Undo);

		const Extent& text_field_dimensions = m_MainTextField->GetDimensions();

		const int top = 2;
		const int bottom = text_field_dimensions.m_Height - 1;
		const int order_list_overview_bottom = bottom - (1 + AuxilaryDataPlayMarkers::MaxPlayMarkers);
		const int player_markers_list_top = order_list_overview_bottom + 1;

		// Create orderlist overview component
		std::shared_ptr<DataSourceTableText> song_view_text_buffer = std::make_shared<DataSourceTableText>(
			OrderListOverviewID,
			256,
			m_DriverInfo->GetAuxilaryDataCollection().GetTableText()
		);

		m_OrderListOverviewComponent = std::make_shared<ComponentOrderListOverview>(
			OrderListOverviewID, 0, 
			undo,
			m_MainTextField, 
			song_view_text_buffer,
			m_OrderListDataSources, 
			m_SequenceDataSources, 
			1, 
			top, 
			order_list_overview_bottom - top, 
			[&](int inEventPosition, bool inStartPlayingFromPosition)
			{
				m_TracksComponent->SetEventPosition(inEventPosition, false); 
				if(inStartPlayingFromPosition)
					DoPlay(inEventPosition);
			}
		);
		m_ComponentsManager->AddComponent(m_OrderListOverviewComponent);

		const Foundation::Rect orderlist_overview_rect = m_OrderListOverviewComponent->GetRect();
		const int tracks_table_x = orderlist_overview_rect.m_Position.m_X + orderlist_overview_rect.m_Dimensions.m_Width + 1;

		// Play markers component
		const int play_markers_width = orderlist_overview_rect.m_Dimensions.m_Width;
		auto play_markers_data_source = std::make_shared<DataSourcePlayMarkers>(m_DriverInfo->GetAuxilaryDataCollection().GetPlayMarkers(), m_DisplayState);

		m_PlayMarkerListComponent = std::make_shared<ComponentStringListSelector>(
			PlayMarkerListID, 0,
			undo, 
			play_markers_data_source, 
			m_MainTextField, 
			1, 
			player_markers_list_top, 
			play_markers_width, 
			bottom - player_markers_list_top, 
			1, 
			0
		);
		m_PlayMarkerListComponent->SetColors(ToColor(UserColor::MarkerListBackground), ToColor(UserColor::MarkerListCursorFocus), ToColor(UserColor::MarkerListCursorNoFocus));
		m_PlayMarkerListComponent->SetColors(ToColor(UserColor::MarkerListText));
		m_PlayMarkerListComponent->SetSelectionCallback([&](bool inOnDoubleClick)
		{
			if (!inOnDoubleClick)
				DoMoveToEventPositionOfSelectedMarker();
			else
				DoPlayFromSelectedMarker();
		});
		m_ComponentsManager->AddComponent(m_PlayMarkerListComponent);

		// Create the tracks component for editing orderlist and sequence data
		m_TracksComponent = std::make_shared<ComponentTracks>(
			TracksTableID, 0, 
			undo, 
			m_TracksDataSource, 
			m_MainTextField, 
			m_DriverInfo->GetAuxilaryDataCollection(), 
			m_EditState,
			tracks_table_x, 
			top, 
			bottom - top);
		m_ComponentsManager->AddComponent(m_TracksComponent);

		// Rect for tables
		const Foundation::Rect tracks_rect = m_TracksComponent->GetRect();
		const int left = tracks_rect.m_Position.m_X + tracks_rect.m_Dimensions.m_Width + 1;
		const int right = text_field_dimensions.m_Width - 1;

		const Foundation::Rect tables_rect = { { left, top }, { right - left, bottom - top } };

		// Create tables as configured by the driver
		const std::vector<DriverInfo::TableDefinition>& table_definitions = m_DriverInfo->GetTableDefinitions();

		int command_and_instrument_table_ref = 0;
		
		int highest_table = 0;
		int widest_table = 0;

		int current_x = 0;
		int current_y = 0;
		int current_y_offset = 0;
		int last_table_height = 0;

		for (const auto& table_definition : table_definitions)
		{
			const bool table_has_text = table_definition.m_TextFieldSize > 0;
			const int table_width = ComponentTableRowElements::GetWidthFromColumns(table_definition.m_ColumnCount) + (table_has_text ? (1 + table_definition.m_TextFieldSize) : 0);
			const int table_height = table_definition.m_VisibleRowCount;

			if (table_definition.m_PropertyLayoutVertically)
			{
				current_y_offset += last_table_height + 2;
			}
			else
			{
				// Change position
				current_y_offset = 0;
				current_x += widest_table > 0 ? (widest_table + 2) : 0;
				widest_table = 0;
			}

			if (table_width + current_x > tables_rect.m_Dimensions.m_Width || command_and_instrument_table_ref == 2)
			{
				current_x = 0;
				current_y += highest_table + 2;
				current_y_offset = 0;
				highest_table = 0;
				widest_table = 0;
			}

			// Create data source
			std::shared_ptr<DataSourceTable> table_data_source = DriverUtils::CreateTableDataSource(table_definition, m_CPUMemory);

			// Create table
			std::shared_ptr<ComponentTableRowElements> table = [&]() -> std::shared_ptr<ComponentTableRowElements>
			{
				if (table_has_text)
				{
					std::shared_ptr<DataSourceTableText> text_buffer = std::make_shared<DataSourceTableText>(
						table_definition.m_ID,
						table_definition.m_RowCount,
						m_DriverInfo->GetAuxilaryDataCollection().GetTableText()
					);

					return std::make_shared<ComponentTableRowElementsWithText>(
						table_definition.m_ID,
						0,
						undo,
						table_data_source,
						text_buffer,
						m_MainTextField,
						current_x + tables_rect.m_Position.m_X,
						current_y + current_y_offset + tables_rect.m_Position.m_Y + 1,
						table_height,
						table_definition.m_TextFieldSize,
						table_definition.m_PropertyIndexAsContinuousMemory
					);
				}
				else
				{
					return std::make_shared<ComponentTableRowElements>(
						table_definition.m_ID,
						0,
						undo,
						table_data_source,
						m_MainTextField,
						current_x + tables_rect.m_Position.m_X,
						current_y + current_y_offset + tables_rect.m_Position.m_Y + 1,
						table_height,
						table_definition.m_PropertyIndexAsContinuousMemory
					);
				}
			}();

			// Enable insert and delete on the table
			if (table_definition.m_PropertyEnabledInsertDelete)
			{
				table->EnableInsertDelete(table_definition.m_PropertyEnabledInsertDelete);
				table->SetInsertDeleteCallback([&](int inTableID, int inIndexPre, int inIndexPost)
				{
					ExecuteTableInsertDeleteRule(inTableID, inIndexPre, inIndexPost);
				});
			}

			// Pass on color rule
			if (table_definition.m_ColorRuleID != 0xff)
			{
				DriverInfo::TableColorRules color_rules = m_DriverInfo->GetTableColorRules()[table_definition.m_ColorRuleID];

				if (m_ConvertLegacyDriverTableDefaultColors)
				{
					for (auto& color_rule : color_rules.m_Rules)
					{
						switch (static_cast<Color>(color_rule.m_BackgroundColor))
						{
						case Color::LightGreen:
							color_rule.m_BackgroundColor = static_cast<unsigned char>(UserColor::DriverColorSet) + 0x40;
							break;
						case Color::LightYellow:
							color_rule.m_BackgroundColor = static_cast<unsigned char>(UserColor::DriverColorJump) + 0x40;
							break;
						default:
							break;
						}
					}
				}

				table->SetColorRules(color_rules);
			}

			// Pass on action rule
			if (table_definition.m_EnterActionRuleID != 0xff)
			{
				table->SetTableActionRules(m_DriverInfo->GetTableActionRules()[table_definition.m_EnterActionRuleID]);
				table->SetActionRuleExecutionCallback([&](int inTableID, int inActionInput)
				{
					ExecuteTableAction(inTableID, inActionInput);
				});
			}

			// Add spacebar pressed callback
			table->SetSpaceBarPressedCallback([&](bool inPressed, bool inForceApplyCommand) { DoSpaceBarFromTable(inPressed, inForceApplyCommand); });

			// Add the table to the components manager
			m_ComponentsManager->AddComponent(table);

			// Store instrument and command tables
			if (table_definition.m_Type == DriverInfo::TableType::Instruments)
			{
				m_InstrumentTableComponent = table;
				m_InstrumentTableDataSource = table_data_source;

				table->GetSelectedRowChangedEvent().Add(
					nullptr,
					Utility::TDelegate<void(int)>([&](int inSelectedRow) 
					{ 
						m_EditState.SetSelectedInstrument(static_cast<unsigned char>(inSelectedRow)); 
						m_TracksComponent->ForceRefresh();
					})
				);

				//++command_and_instrument_table_ref;
			}
			else if (table_definition.m_Type == DriverInfo::TableType::Commands)
			{
				m_CommandTableComponent = table;
				m_CommandTableDataSource = table_data_source;

				table->GetSelectedRowChangedEvent().Add(
					nullptr,
					Utility::TDelegate<void(int)>([&](int inSelectedRow) 
					{ 
						m_EditState.SetSelectedCommand(static_cast<unsigned char>(inSelectedRow)); 
						m_TracksComponent->ForceRefresh();
					})
				);

				//++command_and_instrument_table_ref;
			}

			// Print table name
			const int label_x = current_x + tables_rect.m_Position.m_X;
			const int label_y = current_y + current_y_offset + tables_rect.m_Position.m_Y;

			m_MainTextField->Print(label_x, label_y, ToColor(UserColor::ScreenEditTableHeadline), table_definition.m_Name);

			// Highlight context letter
			const auto it = std::find_if(m_KeyTableIDPairs.begin(), m_KeyTableIDPairs.end(), [&](const auto& data) { return data.m_TableID == table_definition.m_ID; });
			if (it != m_KeyTableIDPairs.end())
				m_MainTextField->ColorArea(ToColor(UserColor::ScreenEditTableHeadlineHotkeyLetter), label_x + it->m_LetterIndex, label_y, 1, 1);

			if (table_height > highest_table + current_y_offset)
				highest_table = table_height + current_y_offset;
			if (table_width > widest_table)
				widest_table = table_width;

			last_table_height = table_height;
		}

		// Hook up the orderlist overview to changes made to the orderlists
		for (int i = 0; i < m_TracksDataSource->GetSize(); ++i)
		{
			(*m_TracksDataSource)[i]->GetOrderListChangedEvent().Add(
				reinterpret_cast<void*>(i),
				Utility::TDelegate<void(void)>([&]() { m_OrderListOverviewComponent->ForceRefresh(); })
			);
		}

		// Enable groups
		//m_ComponentsManager->SetGroupEnabledForTabbing(0);
		m_ComponentsManager->SetGroupEnabledForInput(0, true);
		m_ComponentsManager->SetComponentInFocus(m_TracksComponent);
	}


	void ScreenEdit::ExecuteTableInsertDeleteRule(int inTableID, int inIndexPre, int inIndexPost)
	{
		const std::vector<DriverInfo::TableDefinition>& definitions = m_DriverInfo->GetTableDefinitions();

		for (auto& definition : definitions)
		{
			if (definition.m_ID == inTableID)
			{
				if (definition.m_InsertDeleteRuleID == 0xff)
					return;

				const DriverInfo::TableInsertDeleteRules& table_rules = m_DriverInfo->GetTableInsertDeleteRules()[definition.m_InsertDeleteRuleID];
				ExecuteTableInsertDeleteRules(table_rules, inTableID, inIndexPre, inIndexPost);
			}
		}
	}


	void ScreenEdit::ExecuteTableAction(int inTableID, int inActionInput)
	{
		auto* component = m_ComponentsManager->GetComponent(inTableID);

		if (component != nullptr)
		{
			component->ExecuteAction(inActionInput);
			m_ComponentsManager->SetComponentInFocus(inTableID);
		}
	}


	void ScreenEdit::ExecuteTableInsertDeleteRules(const DriverInfo::TableInsertDeleteRules& inTableRules, int inTableSourceID, int inIndexPre, int inIndexPost)
	{
		for (auto& table_rule : inTableRules.m_Rules)
		{
			ComponentBase* component = m_ComponentsManager->GetComponent(table_rule.m_TargetTableID);

			if (component != nullptr)
				component->ExecuteInsertDeleteRule(table_rule, inTableSourceID, inIndexPre, inIndexPost);
		}
	}


	void ScreenEdit::OnDriverPostInitPlayFromEventPos(Emulation::CPUMemory* inCPUMemory, int inEventPosition)
	{
		FOUNDATION_ASSERT(m_DriverInfo != nullptr);

		std::vector<IDriverArchitecture::PlayMarkerInfo> play_marker_info;

		// Get the driver architecture instance
		IDriverArchitecture* driver_architecture = m_DriverInfo->GetDriverArchitecture();

		// Post init only if valid data was computed for the playback state
		if (m_TracksComponent->ComputePlaybackStateFromEventPosition(inEventPosition, play_marker_info))
			driver_architecture->PostInitSetPlaybackIndices(play_marker_info, inCPUMemory, *m_DriverInfo);
	}


	void ScreenEdit::OnDriverPostApplyChannelMuteState(CPUMemory* inCPUMemory, int inTrack)
	{
		bool track_is_muted = m_TracksComponent->IsMuted(inTrack);

		unsigned short sid_offset_address = m_DriverInfo->GetDriverCommon().m_SIDChannelOffsetAddress;
		unsigned char sid_offset_value = !track_is_muted ? (7 * static_cast<unsigned char>(inTrack)) : 0x19;

		(*inCPUMemory)[sid_offset_address + inTrack] = sid_offset_value;
	}


	void ScreenEdit::OnDriverPostApplyAllChannelsMuteState(Emulation::CPUMemory* inCPUMemory)
	{
		for (int i = 0; i < m_DriverInfo->GetMusicData().m_TrackCount; ++i)
			OnDriverPostApplyChannelMuteState(inCPUMemory, i);
	}


	void ScreenEdit::OnDriverPostClearMuteState(Emulation::CPUMemory* inCPUMemory)
	{
		for (int i = 0; i < m_DriverInfo->GetMusicData().m_TrackCount; ++i)
		{
			unsigned short sid_offset_address = m_DriverInfo->GetDriverCommon().m_SIDChannelOffsetAddress;
			unsigned char sid_offset_value = 7 * static_cast<unsigned char>(i);

			(*inCPUMemory)[sid_offset_address + i] = sid_offset_value;
		}
	}


	void ScreenEdit::OnDriverPostUpdate(Emulation::CPUMemory* inCPUMemory)
	{
		switch (m_DriverState.GetPlayState())
		{
		case DriverState::PlayState::Playing:
			{
				const unsigned short tempo_counter_address = m_DriverInfo->GetDriverCommon().m_TempoCounterAddress;

				if (tempo_counter_address != 0)
				{
					const unsigned char tempo_counter_value = (*inCPUMemory)[tempo_counter_address];

					if (tempo_counter_value == 0)
						++m_PlaybackCurrentEventPos;

					if (m_PlaybackCurrentEventPos >= m_TracksComponent->GetMaxEventPosition())
						m_PlaybackCurrentEventPos = m_TracksComponent->GetLoopEventPosition();
				}
			}
			break;
		case DriverState::PlayState::Playing_Input:
			{
				const DriverInfo::DriverCommon& driver_common = m_DriverInfo->GetDriverCommon();
				unsigned short input_play_track = [&]()
				{
					int track = m_TracksComponent->GetFocusTrackIndex();

					if (track < 0)
						track = 0;
					if (track >= m_DriverInfo->GetMusicData().m_TrackCount)
						track = m_DriverInfo->GetMusicData().m_TrackCount - 1;

					return static_cast<unsigned short>(track);
				}();

				// Keep the tempo counter well away from updating sequence and tracks
				const unsigned short tempo_counter_addres = driver_common.m_TempoCounterAddress;
				(*inCPUMemory)[tempo_counter_addres] = 0x0f;

				// If changes are found relating to note inputs, pass them on to the driver
				const DriverState::PlayNote& play_note = m_DriverState.GetPlayNote();
				if (m_LastPlayNote != play_note.m_Note)
				{
					int new_note = play_note.m_Note;
					const unsigned char note_event_trigger_sync_value = driver_common.m_NoteEventTriggerSyncValue;

					for (int i = 0; i < m_DriverInfo->GetMusicData().m_TrackCount; ++i)
					{
						if (i == input_play_track)
						{
							// This could and should probably be executed from the driver architecture implementation
							(*inCPUMemory)[driver_common.m_NextNoteAddress + input_play_track] = static_cast<unsigned char>(new_note == -1 ? 0 : new_note);
							(*inCPUMemory)[driver_common.m_NextInstrumentAddress + input_play_track] = static_cast<unsigned char>(play_note.m_Instrument | 0x80);

							if (play_note.m_ApplyCommand)
								(*inCPUMemory)[driver_common.m_NextCommandAddress + input_play_track] = static_cast<unsigned char>(play_note.m_Command | 0x80);

							(*inCPUMemory)[driver_common.m_NextNoteIsTiedAddress + input_play_track] = m_LastPlayNote != -1 ? 1 : 0;
							(*inCPUMemory)[driver_common.m_TriggerSyncAddress + input_play_track] = note_event_trigger_sync_value;
						}
						else
						{
							(*inCPUMemory)[driver_common.m_NextNoteAddress + i] = 0;
							(*inCPUMemory)[driver_common.m_NextNoteIsTiedAddress + i] = 1;
							(*inCPUMemory)[driver_common.m_TriggerSyncAddress + i] = note_event_trigger_sync_value;
						}
					}

					m_LastPlayNote = new_note;
				}
			}
			break;
		default:
			break;
		}
	}

	void ScreenEdit::SetStatusPlaying(bool inIsPlaying)
	{
		const bool is_playing = m_DriverState.GetPlayState() == Editor::DriverState::PlayState::Playing;

		if (is_playing != inIsPlaying)
		{
			auto* flight_recorder = m_ExecutionHandler->GetFlightRecorder();
			flight_recorder->Lock();

			if (inIsPlaying)
			{
				// Enable flight recorder
				flight_recorder->Reset();
				flight_recorder->SetRecording(true);

				m_TracksComponent->CancelFocusModeOrderList();
				m_EditState.SetPreventSequenceEdit(m_EditState.IsFollowPlayMode());

				m_ExecutionHandler->GetFlightRecorder()->Reset();
				m_ExecutionHandler->GetFlightRecorder()->SetRecording(true);

				m_DriverState.SetPlayState(Editor::DriverState::PlayState::Playing);
				m_PlayTimerTicks = 0;
				m_PlayTimerSeconds = 0;
			}
			else
			{
				flight_recorder->SetRecording(false);
				m_EditState.SetPreventSequenceEdit(false);

				m_DriverState.SetPlayState(Editor::DriverState::PlayState::Stopped);
				m_ComponentsManager->Update(0, m_CPUMemory);
			}

			flight_recorder->Unlock();
		}
	}


	void ScreenEdit::SetStatusPlayingInput()
	{
		if (m_DriverState.GetPlayState() == DriverState::PlayState::Stopped)
		{
			m_ExecutionHandler->QueueInit(0);

			m_DriverState.SetPlayState(DriverState::PlayState::Playing_Input);
			m_LastPlayNote = -1;
		}
	}


	void ScreenEdit::RestoreSIDOffsetData()
	{
		if (m_DriverInfo->IsValid())
		{
			m_CPUMemory->Lock();

			for (int i = 0; i < m_DriverInfo->GetMusicData().m_TrackCount; ++i)
			{
				unsigned short sid_offset_address = m_DriverInfo->GetDriverCommon().m_SIDChannelOffsetAddress;
				unsigned char sid_offset_value = 7 * static_cast<unsigned char>(i);

				(*m_CPUMemory)[sid_offset_address + i] = sid_offset_value;
			}

			m_CPUMemory->Unlock();
		}
	}


	void ScreenEdit::ConfigureKeyHooks()
	{
		using namespace Utility;

		m_KeyHooks.clear();

		m_KeyHooks.push_back({ "Key.ScreenEdit.Stop", m_KeyHookStore, [&]()
		{
			if (IsPlaying())
			{
				DoStop();
				return true;
			}

			return false;
		} });

		m_KeyHooks.push_back({"Key.ScreenEdit.Play", m_KeyHookStore, [&]()
		{
			if (IsPlaying())
				DoStop();
			else
				DoPlay();

			return true;
		} });
 
		m_KeyHooks.push_back({ "Key.ScreenEdit.PlayFromMarker", m_KeyHookStore, [&]()
		{
			if (IsPlaying())
				DoStop();
			else
				DoPlayFromSelectedMarker();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.PlayFromLast", m_KeyHookStore, [&]()
		{
			if (IsPlaying())
				DoStop();
			else
				DoPlay(m_TracksComponent->GetEventPositionAtTopOfCurrentSequence());

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.PlayFromCursor", m_KeyHookStore, [&]()
		{
			if (IsPlaying())
				DoStop();
			else
				DoPlay(m_TracksComponent->GetEventPosition());

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.OpenUtilitiesDialog", m_KeyHookStore, [&]()
		{
			if(IsPlaying())
				DoStop();

			DoUtilitiesDialog();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.OpenOptionsDialog", m_KeyHookStore, [&]()
		{
			if (IsPlaying())
				DoStop();

			DoOptionsDialog();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleOverlay", m_KeyHookStore, [&]()
		{
			m_ToggleShowOverlay();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.OctaveDown", m_KeyHookStore, [&]()
		{ 
			DoOctaveChange(false); 
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.OctaveUp", m_KeyHookStore, [&]()
		{ 
			DoOctaveChange(true); 
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleSIDModel", m_KeyHookStore, [&]()
		{ 
			DoToggleSIDModelAndRegion(false); 
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleRegion", m_KeyHookStore, [&]()
		{ 
			DoToggleSIDModelAndRegion(true); 
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.LoadSong", m_KeyHookStore, [&]()
		{ 
			DoStop();
			DoLoadSong();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.LoadInstrument", m_KeyHookStore, [&]()
		{ 
			DoStop();
			DoLoadInstrument();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ImportSong", m_KeyHookStore, [&]()
		{ 
			DoStop();
			DoLoadImportSong();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.SaveSong", m_KeyHookStore, [&]()
		{ 
			DoStop();
			DoSaveSong();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.SaveInstrument", m_KeyHookStore, [&]()
		{ 
			DoStop();
			DoSaveInstrument();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleFlightRecorderOverlay", m_KeyHookStore, [&]()
		{
			m_OverlayFlightRecorder->SetEnabled(!m_OverlayFlightRecorder->IsEnabled());

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleDebugView", m_KeyHookStore, [&]()
		{
			m_DebugViews->SetEnabled(!m_DebugViews->IsEnabled());

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleMuteChannel1", m_KeyHookStore, [&]()
		{
			DoToggleMute(0);

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleMuteChannel2", m_KeyHookStore, [&]()
		{
			DoToggleMute(1);

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleMuteChannel3", m_KeyHookStore, [&]()
		{
			DoToggleMute(2);

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.SetMarker", m_KeyHookStore, [&]()
		{
			const int selected_marker = m_PlayMarkerListComponent->GetSelectionIndex();
			auto& play_markers = m_DriverInfo->GetAuxilaryDataCollection().GetPlayMarkers();

			play_markers.SetPlayMarkerEventPosition(selected_marker, m_TracksComponent->GetEventPosition());

			m_PlayMarkerListComponent->ForceRefresh();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.GotoMarker", m_KeyHookStore, [&]()
		{
			DoMoveToEventPositionOfSelectedMarker();
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.QuickSave", m_KeyHookStore, [&]()
		{
			m_ExecutionHandler->QueueStop();
			SetStatusPlaying(false);

			m_PlaybackCurrentEventPos = -1;
			m_CPUMemory->Lock();
			m_InstrumentTableDataSource->PushDataToSource();
			m_CPUMemory->Unlock();

			m_QuickSaveCallback();

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.Undo", m_KeyHookStore, [&]()
		{
			if (m_Undo->HasUndoStep())
			{
				const int target_component_id = m_Undo->DoUndo(*m_CursorControl);
				if (target_component_id >= 0)
					m_ComponentsManager->PullDataFromAllSources(true);
			}

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.Redo", m_KeyHookStore, [&]()
		{
			if (m_Undo->HasRedoStep())
			{
				const int target_component_id = m_Undo->DoRedo(*m_CursorControl);
				if (target_component_id >= 0)
					m_ComponentsManager->PullDataFromAllSources(true);
			}

			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.DecrementEventHighlightInterval", m_KeyHookStore, [&]()
		{
			DoDecrementHighlightIntervalOrOffset(false);
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.DecrementEventHighlightOffset", m_KeyHookStore, [&]()
		{
			DoDecrementHighlightIntervalOrOffset(true);
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.IncrementEventHighlightInterval", m_KeyHookStore, [&]()
		{
			DoIncrementHighlightIntervalOrOffset(false);
			return true;
		} });

		m_KeyHooks.push_back({ "Key.ScreenEdit.IncrementEventHighlightOffset", m_KeyHookStore, [&]()
		{
			DoIncrementHighlightIntervalOrOffset(true);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleFollowPlay", m_KeyHookStore, [&]()
		{
			DoToggleFollowPlay();

			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleDisplayHexUppercase", m_KeyHookStore, [&]()
		{
			m_DisplayState.SetIsHexUppercase(!m_DisplayState.IsHexUppercase());
			m_ComponentsManager->ForceRefresh();

			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.IncrementInstrumentIndex", m_KeyHookStore, [&]()
		{
			DoIncrementInstrumentIndex();
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.DecrementInstrumentIndex", m_KeyHookStore, [&]()
		{
			DoDecrementInstrumentIndex();
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.IncrementCommandIndex", m_KeyHookStore, [&]()
		{
			DoIncrementCommandIndex();
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.DecrementCommandIndex", m_KeyHookStore, [&]()
		{
			DoDecrementCommandIndex();
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.SetOrderlistLoopPointAll", m_KeyHookStore, [&]()
		{
			DoSetOrderlistLoopPointAll();
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.Config.Reload", m_KeyHookStore, [&]()
		{
			m_ConfigReconfigure(0);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleColorSchemes", m_KeyHookStore, [&]()
		{
			m_ConfigReconfigure(1);
			return true;
		} });
		m_KeyHooks.push_back({ "Key.ScreenEdit.RefreshColorSchemes", m_KeyHookStore, [&]()
		{
			m_ConfigReconfigure(2);
			return true;
		} });

		// Fast forward key hooks
		m_FastForwardKeyHooks.push_back({ "Key.ScreenEdit.FastForward", m_KeyHookStore, [&]()
		{
			m_FastForwardFactor = 3;
			return true;
		} });
		m_FastForwardKeyHooks.push_back({ "Key.ScreenEdit.FasterForward", m_KeyHookStore, [&]()
		{
			m_FastForwardFactor = 100;
			return true;
		} });

/*		m_KeyHooks.push_back({ "Key.ScreenEdit.ToggleRecordOutput", SDLK_r, Keyboard::Control, [&]()
		{
			if (m_ExecutionHandler->IsWritingOutputToFile())
				m_ExecutionHandler->StopWriteOutputToFile();
			else
				m_ExecutionHandler->StartWriteOutputToFile("C:\\Temp\\sf2output.wav");
			return true;
		} }); */


	}

	void ScreenEdit::ConfigureDynamicKeyHooks()
	{
		using namespace Utility;

		auto focus_toggle_function = [&](DynamicKeysContext& inContext)
		{
			const auto it = std::find_if(m_KeyTableIDPairs.begin(), m_KeyTableIDPairs.end(), [&](const auto& data) { return data.m_AssociatedKeyCode == inContext.m_KeyCode; });
			if (it != m_KeyTableIDPairs.end())
			{
				if (!inContext.m_ComponentsManager.IsComponentInFocus(it->m_TableID))
					inContext.m_ComponentsManager.SetComponentInFocus(it->m_TableID);
				else
					inContext.m_ComponentsManager.SetComponentInFocus(TracksTableID);

				return true;
			}

			return false;
		};

		m_KeyTableIDPairs.clear();

		const std::vector<DriverInfo::TableDefinition>& table_definitions = m_DriverInfo->GetTableDefinitions();
		for (const auto& table_definition : table_definitions)
		{
			int associated_character_index = [&](const std::string& inTableName)
			{
				const size_t string_length = inTableName.length();
				const char* c_string = inTableName.c_str();

				for (size_t i = 0; i < string_length; ++i)
				{
					const SDL_Keycode key_code = static_cast<SDL_Keycode>(std::tolower(c_string[i]));

					const auto it = std::find_if(m_KeyTableIDPairs.begin(), m_KeyTableIDPairs.end(), [&](const auto& data) { return data.m_AssociatedKeyCode == key_code; });
					if (it == m_KeyTableIDPairs.end())
						return static_cast<int>(i);
				}
				
				return -1;
			}(table_definition.m_Name);

			if (associated_character_index >= 0)
			{
				const char* c_string = table_definition.m_Name.c_str();
				const SDL_Keycode associated_key_code = static_cast<SDL_Keycode>(std::tolower(c_string[associated_character_index]));

				m_KeyTableIDPairs.push_back({ associated_key_code, associated_character_index, table_definition.m_ID });
				m_DynamicKeyHooks.push_back({ table_definition.m_Name, { associated_key_code, Keyboard::Alt }, focus_toggle_function });
			}
		}
	}
}

