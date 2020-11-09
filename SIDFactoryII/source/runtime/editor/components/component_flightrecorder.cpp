#include "component_flightrecorder.h"

#include "runtime/editor/datasources/datasource_flightrecorder.h"
#include "runtime/editor/display_state.h"
#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/keyboard_utils.h"
#include "runtime/editor/cursor_control.h"
#include "runtime/environmentdefines.h"
#include "utils/usercolors.h"

#include "SDL_keycode.h"
#include "foundation/base/assert.h"

using namespace Foundation;
using namespace Utility;

namespace Editor
{
	ComponentFlightRecorder::ComponentFlightRecorder(int inID, int inGroupID, Undo* inUndo, TextField* inTextField, int inX, int inY, int inHeight, std::shared_ptr<DataSourceFlightRecorder>& inDataSource)
		: ComponentBase(inID, inGroupID, inUndo, inTextField, inX, inY, 40, inHeight)
		, m_DataSource(inDataSource)
		, m_CursorPos(0)
		, m_MaxCursorPos(static_cast<unsigned int>(m_DataSource->GetSize()) - inHeight)
	{
		FOUNDATION_ASSERT(inTextField != nullptr);
		m_RequireRefresh = true;
	}


	ComponentFlightRecorder::~ComponentFlightRecorder()
	{

	}


	void ComponentFlightRecorder::SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl)
	{
		m_HasControl = true;
		inCursorControl.SetEnabled(false);
	}


	bool ComponentFlightRecorder::ConsumeInput(const Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		if (m_HasControl && !inKeyboard.GetKeyEventList().empty())
		{
			bool consume_input = false;

			// Get key events
			for (const auto& key_event : inKeyboard.GetKeyEventList())
			{
				switch (key_event)
				{
				case SDLK_UP:
					m_CursorPos--;
					if (m_CursorPos > m_MaxCursorPos)
						m_CursorPos = 0;

					consume_input = true;
					break;

				case SDLK_DOWN:
					m_CursorPos++;
					if (m_CursorPos > m_MaxCursorPos)
						m_CursorPos = m_MaxCursorPos;

					consume_input = true;
					break;
				case SDLK_PAGEUP:
					m_CursorPos -= 8;
					if (m_CursorPos > m_MaxCursorPos)
						m_CursorPos = 0;

					consume_input = true;
					break;
				case SDLK_PAGEDOWN:
					m_CursorPos += 8;
					if (m_CursorPos > m_MaxCursorPos)
						m_CursorPos = m_MaxCursorPos;

					consume_input = true;
					break;
				case SDLK_HOME:
					m_CursorPos = 0;

					consume_input = true;
					break;
				case SDLK_END:
					m_CursorPos = m_MaxCursorPos;

					consume_input = true;
					break;
				default:
					break;
				}
			}

			return consume_input;
		}

		return false;
	}


	bool ComponentFlightRecorder::ConsumeInput(const Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager)
	{
		return false;
	}


	void ComponentFlightRecorder::ConsumeNonExclusiveInput(const Mouse& inMouse)
	{
		Point scroll_wheel = inMouse.GetWheelDelta();

		if (scroll_wheel.m_Y != 0)
		{
			Point screen_position = inMouse.GetPosition();
			if (ContainsPosition(screen_position))
			{
				int cursor_pos = static_cast<int>(m_CursorPos);
				int change = scroll_wheel.m_Y;

				cursor_pos += change;

				if (cursor_pos < 0)
					cursor_pos = 0;
				if (cursor_pos > static_cast<int>(m_MaxCursorPos))
					cursor_pos = static_cast<int>(m_MaxCursorPos);

				if (change != 0)
					m_CursorPos = static_cast<unsigned int>(cursor_pos);
			}
		}
	}


	void ComponentFlightRecorder::Refresh(const DisplayState& inDisplayState)
	{
		if (m_RequireRefresh && m_TextField->IsEnabled())
		{
			const bool is_uppercase = inDisplayState.IsHexUppercase();

			const Color color_gate_off = ToColor(UserColor::FlightRecorderGateOff);
			const Color color_gate_on = ToColor(UserColor::FlightRecorderGateOn);
			const Color color_filter_and_volume = ToColor(UserColor::FlightRecorderFilterAndVolume);
			const Color color_cpu_usage_low = ToColor(UserColor::FlightRecorderCPUUsageLow);
			const Color color_cpu_usage_medium = ToColor(UserColor::FlightRecorderCPUUsageMedium);
			const Color color_cpu_usage_high = ToColor(UserColor::FlightRecorderCPUUsageHigh);
			const Color color_desc = ToColor(UserColor::FlightRecorderDesc);

			m_DataSource->Lock();

			auto print_channel = [&](int x, int y, int channel, const Emulation::FlightRecorder::Frame& frame_data)
			{
				const int sid_index = 7 * channel;

				m_TextField->PrintHexValue(x, y, is_uppercase, frame_data.m_DriverSync[channel]);
				
				unsigned short FREQ = (static_cast<unsigned short>(frame_data.m_SIDData[sid_index + 1]) << 8) | frame_data.m_SIDData[sid_index + 0];
				unsigned short PLSW = (static_cast<unsigned short>(frame_data.m_SIDData[sid_index + 3]) << 8) | frame_data.m_SIDData[sid_index + 2];
				unsigned short ADSR = (static_cast<unsigned short>(frame_data.m_SIDData[sid_index + 5]) << 8) | frame_data.m_SIDData[sid_index + 6];
				
				Color color = (frame_data.m_SIDData[sid_index + 4] & 1) == 0 ? color_gate_off : color_gate_on;

				m_TextField->PrintHexValue(x + 3, y, color, is_uppercase, FREQ);
				m_TextField->PrintHexValue(x + 8, y, color, is_uppercase, PLSW);
				m_TextField->PrintHexValue(x + 13, y, color, is_uppercase, frame_data.m_SIDData[sid_index + 4]);
				m_TextField->PrintHexValue(x + 16, y, color, is_uppercase, ADSR);
			};

			auto print_filter = [&](int x, int y, const Emulation::FlightRecorder::Frame& frame_data)
			{
				unsigned short CUTOFF = (static_cast<unsigned short>(frame_data.m_SIDData[0x16]) << 3) | (frame_data.m_SIDData[0x15] & 7);
				unsigned char RES_SEL = frame_data.m_SIDData[0x17];
				unsigned char BANDPASS_VOL = frame_data.m_SIDData[0x18];

				Color color = color_filter_and_volume;

				m_TextField->PrintHexValue(x, y, color, is_uppercase, CUTOFF);
				m_TextField->PrintHexValue(x + 5, y, color, is_uppercase, RES_SEL);
				m_TextField->PrintHexValue(x + 8, y, color, is_uppercase, BANDPASS_VOL);
			};

			m_TextField->Print(2, 1, color_desc, "Frame Cycl SL  SC  Sy Freq Puls Wf ADSR  Sy Freq Puls Wf ADSR  Sy Freq Puls Wf ADSR  CutO RS BV");

			const int x = 2;

			for (int i = 0; i < m_Dimensions.m_Height - 4; ++i)
			{
				const int y = i + 3;

				const int index = m_CursorPos + i;

				if (index < m_DataSource->GetSize())
				{
					const Emulation::FlightRecorder::Frame& frame_data = (*m_DataSource)[index];
					unsigned short frame_number = static_cast<unsigned short>(frame_data.m_nFrameNumber);

					const unsigned char scan_lines = static_cast<unsigned char>(frame_data.m_nCyclesSpend / EMULATION_CYCLES_PER_SCANLINE_PAL);

					Color cycle_color = scan_lines < 0x10 ? color_cpu_usage_low : (scan_lines < 0x18 ? color_cpu_usage_medium : color_cpu_usage_high);

					m_TextField->PrintHexValue(x, y, is_uppercase, frame_number);
					m_TextField->PrintChar(x + 4, y, ':');
					m_TextField->PrintHexValue(x + 6, y, cycle_color, is_uppercase, static_cast<unsigned short>(frame_data.m_nCyclesSpend));
					m_TextField->PrintHexValue(x + 11, y, cycle_color, is_uppercase, scan_lines);
					m_TextField->PrintHexValue(x + 15, y, is_uppercase, frame_data.m_TempoCounter);

					print_channel(x + 19, y, 0, frame_data);
					print_channel(x + 41, y, 1, frame_data);
					print_channel(x + 63, y, 2, frame_data);

					print_filter(x + 85, y, frame_data);
				}
			}

			m_DataSource->Unlock();
		}
	}


	void ComponentFlightRecorder::HandleDataChange()
	{
		if (m_HasDataChange)
			m_HasDataChange = false;
	}


	void ComponentFlightRecorder::PullDataFromSource()
	{

	}


	void ComponentFlightRecorder::ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost)
	{
	}

	
	void ComponentFlightRecorder::ExecuteAction(int inActionInput)
	{
	}
}