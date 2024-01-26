#include "status_bar_edit.h"
#include "foundation/graphics/textfield.h"
#include "foundation/graphics/color.h"
#include "runtime/execution/executionhandler.h"
#include "utils/usercolors.h"
#include <string>

using namespace Utility;
using namespace Foundation;

namespace Editor
{
	StatusBarEdit::StatusBarEdit
	(
			TextField* inTextField,
			const EditState& inEditState,
			const DriverState& inDriverState,
			const AuxilaryDataCollection& inAuxilaryDataCollection,
			const Emulation::ExecutionHandler& inExecutionHandler,
			std::function<void(Mouse::Button, int)> inOctaveMousePressCallback,
			std::function<void(Mouse::Button, int)> inSharpFlatMousePressCallback,
			std::function<void(Mouse::Button, int)> inSIDMousePressCallback,
			std::function<void(Mouse::Button, int)> inOutputDevicePressCallback,
			std::function<void(Mouse::Button, int)> inContextHighlightMousePressCallback,
			std::function<void(Mouse::Button, int)> inFollowPlayerMousePressCallback
	)
		: StatusBar(inTextField)
		, m_EditState(inEditState)
		, m_DriverState(inDriverState)
		, m_AuxilaryDataPlayMarkers(inAuxilaryDataCollection)
		, m_ExecutionHandler(inExecutionHandler)
	{
		m_TextSectionOctave = std::make_shared<TextSection>(12, inOctaveMousePressCallback);
		m_TextSectionSharpFlat = std::make_shared<TextSection>(15, inSharpFlatMousePressCallback);
		m_TextSectionSID = std::make_shared<TextSection>(19, inSIDMousePressCallback);
		m_TextSectionContextHighlight = std::make_shared<TextSection>(18, inContextHighlightMousePressCallback);
		m_TextSectionFollowPlay = std::make_shared<TextSection>(15, inFollowPlayerMousePressCallback);
		m_TextSectionOutputDevice = std::make_shared<TextSection>(17, inOutputDevicePressCallback);

		m_TextSectionList.push_back(m_TextSectionOctave);
		m_TextSectionList.push_back(m_TextSectionSharpFlat);
		m_TextSectionList.push_back(m_TextSectionSID);
		m_TextSectionList.push_back(m_TextSectionContextHighlight);
		m_TextSectionList.push_back(m_TextSectionFollowPlay);
		m_TextSectionList.push_back(m_TextSectionOutputDevice);
	}

	
	StatusBarEdit::~StatusBarEdit()
	{

	}
	
	void StatusBarEdit::UpdateInternal(int inDeltaTick, bool inNeedUpdate)
	{
		if (m_CachedEditState != m_EditState || inNeedUpdate)
		{
			std::string sequence_highlight_onoff = (m_EditState.IsSequenceHighlightingEnabled() ? "ON" : "OFF");
			std::string follow_play_onoff = (m_EditState.IsFollowPlayMode() ? "ON" : "OFF");
			m_TextSectionOctave->SetText(" Octave: " + std::to_string(m_EditState.GetOctave()));
			m_TextSectionContextHighlight->SetText(" Highlights: " + sequence_highlight_onoff);
			m_TextSectionFollowPlay->SetText(" Follow: " + follow_play_onoff);

			m_CachedEditState = m_EditState;
			m_NeedRefresh = true;
		}

		const auto& editor_preferences = m_AuxilaryDataPlayMarkers.GetEditingPreferences();
		const auto& hardware_preferences = m_AuxilaryDataPlayMarkers.GetHardwarePreferences();

		const AuxilaryDataHardwarePreferences::SIDModel sid_model = hardware_preferences.GetSIDModel();
		const AuxilaryDataHardwarePreferences::Region region = hardware_preferences.GetRegion();
		
		if (sid_model != m_CachedSIDModel || region != m_CachedRegion || inNeedUpdate)
		{
			std::string sid_region_string = (region == AuxilaryDataHardwarePreferences::Region::PAL ? "PAL" : "NTSC");
			std::string sid_model_string = (sid_model == AuxilaryDataHardwarePreferences::SIDModel::MOS6581 ? "6581" : "8580");
			
			m_TextSectionSID->SetText(" SID: " + sid_model_string + " (" + sid_region_string + ")");

			m_CachedSIDModel = sid_model;
			m_CachedRegion = region;

			m_NeedRefresh = true;
		}

		const Emulation::ExecutionHandler::OutputDevice outputDevice = m_ExecutionHandler.GetOutputDevice();

		if (outputDevice != m_CachedOutputDevice || inNeedUpdate)
		{
			std::string output_device_string = (outputDevice == Emulation::ExecutionHandler::OutputDevice::ASID ? "ASID" : "RESID");
			m_TextSectionOutputDevice->SetText("Output: " + output_device_string);
			m_CachedOutputDevice = outputDevice;
			m_NeedRefresh = true;
		}

		const AuxilaryDataEditingPreferences::NotationMode notation_mode = editor_preferences.GetNotationMode();
		if (notation_mode != m_CachedNotationMode || inNeedUpdate)
		{
			std::string sharp_or_flat = (notation_mode == AuxilaryDataEditingPreferences::NotationMode::Sharp ? "SHARP" : "FLAT");
			m_TextSectionSharpFlat->SetText(" Mode: " + sharp_or_flat);
			m_CachedNotationMode = notation_mode;

			m_NeedRefresh = true;
		}

		if (m_CachedDriverState != m_DriverState || inNeedUpdate)
		{
			// Just doing this to have it set for sure.. Bit of a hack!
			m_TextColor = ToColor(UserColor::StatusBarText);

			switch (m_DriverState.GetPlayState())
			{
			case DriverState::PlayState::Playing:
				m_BackgroundColor = ToColor(UserColor::StatusBarBackgroundPlaying); // DarkGreen;
				m_BackgroundMouseOverColor = ToColor(UserColor::StatusBarBackgroundPlayingMouseOverRegion); // Color::Green;
				break;
			case DriverState::PlayState::Playing_Input:
				m_BackgroundColor = ToColor(UserColor::StatusBarBackgroundPlayingInput); // DarkYellow;
				m_BackgroundMouseOverColor = ToColor(UserColor::StatusBarBackgroundPlayingInputMouseOverRegion);	// Color::Yellow;
				break;
			case DriverState::PlayState::Stopped:
				m_BackgroundColor = ToColor(UserColor::StatusBarBackgroundStopped);	// Color::Blue;
				m_BackgroundMouseOverColor = ToColor(UserColor::StatusBarBackgroundStoppedMouseOverRegion);	// LightBlue;
				break;
			default:
				m_BackgroundColor = Color::Red;
				m_BackgroundColor = Color::LightRed;
				break;
			}

			m_CachedDriverState = m_DriverState;
			m_NeedRefresh = true;
		}
	}


	void StatusBarEdit::ClearContents()
	{
		m_CachedEditState = EditState();
	}


	void StatusBarEdit::DrawText()
	{
		StatusBar::DrawText();
	}
}

