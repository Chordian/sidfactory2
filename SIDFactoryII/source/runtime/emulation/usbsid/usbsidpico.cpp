#include "usbsidpico.h"

#include "runtime/environmentdefines.h"
#include "utils/configfile.h"
#include "utils/global.h"
#include "utils/logging.h"

#include <cstdlib>
#include <thread>

// Driver headers last, the Windows build pulls in windows.h through them
#include "libraries/usbsid/USBSID.h"
#include "libraries/usbsid/USBSID_Manager.h"

using namespace Utility;

namespace Emulation
{
	namespace
	{
		// Cycles a write takes on the board on top of its wait
		const uint64_t BOARD_WRITE_OVERHEAD = 2;

		// Shortest time between two writes on the board
		const uint64_t BOARD_WRITE_MIN = 7;

		// Waits above this are shortened while the board lags
		const uint64_t CATCH_UP_MIN_WAIT = 4096;

		// Frames fed in one call at most, a longer stall restarts the pacing
		const unsigned int PACE_MAX_FRAMES = 3;
		const double PACE_RESTART_FRAMES = 8.0;

		// Largest wait a cycled write carries
		const uint64_t MAX_WAIT_CYCLES = 0xffff;

		// Free ring bytes required before queueing, one write takes 4
		const int RING_LOW_WATER = 1024;

		// Feed gap after which the board has run dry
		const std::chrono::milliseconds RESYNC_GAP(250);

		// Largest lead time that still fits a cycled write together with a full frame
		const unsigned int LEAD_TIME_MAX_MS = 40;

		const unsigned char SID_REGISTER_COUNT = 0x19;

		// Gates and volume first, the last flush may leave a few writes behind
		const unsigned char SILENCE_ORDER[SID_REGISTER_COUNT] =
		{
			0x04, 0x0b, 0x12, 0x18,
			0x00, 0x01, 0x02, 0x03, 0x05, 0x06,
			0x07, 0x08, 0x09, 0x0a, 0x0c, 0x0d,
			0x0e, 0x0f, 0x10, 0x11, 0x13, 0x14,
			0x15, 0x16, 0x17
		};

		std::vector<std::string> SplitSerials(const std::string& inList)
		{
			std::vector<std::string> serials;
			std::string current;

			for (const char character : inList + ",")
			{
				if (character == ',')
				{
					if (!current.empty())
						serials.push_back(current);
					current.clear();
				}
				else if (character != ' ' && character != '\t')
					current += character;
			}

			return serials;
		}
	}


	USBSid::USBSid()
		: m_Manager(new USBSID_Manager())
		, m_BoardsSelected(false)
		, m_Active(false)
		, m_IsOpen(false)
		, m_PAL(true)
		, m_AllSIDs(false)
		, m_LeadTimeMs(30)
		, m_LeadCycles(0)
		, m_FrameBase(0)
		, m_LastFrameTime(std::chrono::steady_clock::now())
		, m_PaceRunning(false)
		, m_PaceStart(std::chrono::steady_clock::now())
		, m_PacedCycles(0.0)
	{
		const ConfigFile& config = Global::instance().GetConfig();

		m_AllSIDs = GetSingleConfigurationValue<Config::ConfigValueInt>(config, "Playback.USBSID.AllSIDs", 0) != 0;

		const int lead_time = GetSingleConfigurationValue<Config::ConfigValueInt>(config, "Playback.USBSID.LeadTime", 30);
		m_LeadTimeMs = lead_time < 0 ? 0 : (lead_time > static_cast<int>(LEAD_TIME_MAX_MS) ? LEAD_TIME_MAX_MS : static_cast<unsigned int>(lead_time));

		const std::string config_boards = GetSingleConfigurationValue<Config::ConfigValueString>(config, "Playback.USBSID.Boards", std::string(""));
		m_SelectedBoards = SplitSerials(config_boards);
		m_BoardsSelected = !m_SelectedBoards.empty();

		// Entries are <board serial>:<SID number>
		const std::string config_sids = GetSingleConfigurationValue<Config::ConfigValueString>(config, "Playback.USBSID.SIDs", std::string(""));

		for (const std::string& entry : SplitSerials(config_sids))
		{
			const size_t separator = entry.rfind(':');

			if (separator == std::string::npos)
				continue;

			const int sid_number = atoi(entry.c_str() + separator + 1);

			if (sid_number > 0)
				m_SelectedSIDs.push_back({ entry.substr(0, separator), sid_number });
		}

		DetectBoards();
	}


	USBSid::~USBSid()
	{
		m_Active = false;
		Close();
	}

	//----------------------------------------------------------------------------------------------------------------
	// Board detection and selection
	//----------------------------------------------------------------------------------------------------------------

	unsigned int USBSid::DetectBoards()
	{
		m_DetectedBoards.clear();

		// An open board is claimed and can not be enumerated again
		if (m_IsOpen)
		{
			for (const auto& board : m_Manager->Boards())
				m_DetectedBoards.push_back(board.serial);
		}
		else
		{
			for (const auto& device : USBSID_Manager::Enumerate())
				m_DetectedBoards.push_back(device.serial);
		}

		Logging::instance().Info("USBSID-Pico boards detected: %d", static_cast<int>(m_DetectedBoards.size()));

		for (const std::string& serial : m_DetectedBoards)
			Logging::instance().Info("USBSID-Pico board serial: %s", serial.c_str());

		return static_cast<unsigned int>(m_DetectedBoards.size());
	}


	bool USBSid::IsBoardSelectionRequired() const
	{
		return m_DetectedBoards.size() > 1 && !m_BoardsSelected;
	}


	void USBSid::SelectBoards(const std::vector<std::string>& inSerials)
	{
		const bool was_active = m_Active;

		m_Active = false;
		Close();

		m_SelectedBoards = inSerials;
		m_BoardsSelected = true;
		m_SelectedSIDs.clear();

		if (was_active)
			SetActive(true);
	}


	std::vector<USBSid::SIDInfo> USBSid::QuerySIDs()
	{
		std::vector<SIDInfo> sids;

		// The SID list comes from the socket configuration, readable on an open board only
		const bool opened_for_query = !m_IsOpen;

		if (opened_for_query && !Open(!m_BoardsSelected))
			return sids;

		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			const auto& boards = m_Manager->Boards();
			const auto& logical_map = m_Manager->LogicalMap();

			for (size_t i = 0; i < logical_map.size(); ++i)
			{
				const auto& slot = logical_map[i];
				bool selected = false;

				for (const Target& target : m_Targets)
					selected |= target.m_LogicalSID == static_cast<int>(i);

				sids.push_back({ boards[slot.board_index].serial, slot.board_index + 1, slot.local_slot + 1, slot.sid_type, selected });
			}
		}

		if (opened_for_query)
			Close();

		return sids;
	}


	void USBSid::SelectSIDs(const std::vector<SIDInfo>& inSIDs)
	{
		const bool was_active = m_Active;

		m_Active = false;
		Close();

		m_SelectedSIDs.clear();
		m_SelectedBoards.clear();

		for (const SIDInfo& sid : inSIDs)
		{
			m_SelectedSIDs.push_back({ sid.m_BoardSerial, sid.m_SIDNumber });

			bool known_board = false;

			for (const std::string& serial : m_SelectedBoards)
				known_board |= serial == sid.m_BoardSerial;

			if (!known_board)
				m_SelectedBoards.push_back(sid.m_BoardSerial);
		}

		m_BoardsSelected = true;

		if (was_active)
			SetActive(true);
	}

	//----------------------------------------------------------------------------------------------------------------
	// Output control
	//----------------------------------------------------------------------------------------------------------------

	bool USBSid::SetActive(bool inActive)
	{
		if (inActive == m_Active)
			return m_Active;

		if (inActive)
		{
			if (!m_IsOpen && !Open())
				return false;

			Resync();
			m_Active = true;
		}
		else
		{
			m_Active = false;
			Silence();
		}

		return m_Active;
	}


	void USBSid::SetPAL(bool inPAL)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (m_PAL == inPAL && m_LeadCycles != 0)
			return;

		m_PAL = inPAL;

		if (m_IsOpen)
			ApplyClockRate();
	}


	void USBSid::Resync()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		for (BoardState& state : m_BoardStates)
			state.m_Synced = false;

		m_PaceRunning = false;
	}


	void USBSid::Silence()
	{
		if (!m_IsOpen)
			return;

		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			QueueSilence();
		}

		// One flush sends a single packet, repeat until the ring is empty
		for (int i = 0; i < 4; ++i)
		{
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				m_Manager->FlushAll();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}

	//----------------------------------------------------------------------------------------------------------------
	// Frame feed
	//----------------------------------------------------------------------------------------------------------------

	unsigned int USBSid::FramesDue(unsigned int inCyclesInFrame)
	{
		if (!m_Active || inCyclesInFrame == 0)
			return 0;

		std::lock_guard<std::mutex> lock(m_Mutex);

		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		const double clock_rate = m_PAL ? EMULATION_CYCLES_PER_SECOND_PAL : EMULATION_CYCLES_PER_SECOND_NTSC;

		if (!m_PaceRunning)
		{
			m_PaceRunning = true;
			m_PaceStart = now;
			m_PacedCycles = 0.0;
		}

		const double elapsed_cycles = std::chrono::duration<double>(now - m_PaceStart).count() * clock_rate;

		// Too far behind to catch up: restart pacing and the board timeline
		if (elapsed_cycles - m_PacedCycles > PACE_RESTART_FRAMES * inCyclesInFrame)
		{
			m_PaceStart = now;
			m_PacedCycles = 0.0;

			for (BoardState& state : m_BoardStates)
				state.m_Synced = false;

			m_PacedCycles += inCyclesInFrame;
			return 1;
		}

		unsigned int frames = 0;

		while (m_PacedCycles <= elapsed_cycles && frames < PACE_MAX_FRAMES)
		{
			m_PacedCycles += inCyclesInFrame;
			++frames;
		}

		return frames;
	}


	void USBSid::BeginFrame()
	{
		if (!m_Active)
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);

		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		if (now - m_LastFrameTime > RESYNC_GAP)
		{
			for (BoardState& state : m_BoardStates)
				state.m_Synced = false;
		}

		m_LastFrameTime = now;

		// More than two frames waiting in the driver ring: the board lags behind the timeline
		for (BoardState& state : m_BoardStates)
		{
			const size_t ring_used = static_cast<size_t>(USBSID_NS::default_ring_size - 1 - m_Manager->RingFreeBytes(state.m_LogicalSID));
			state.m_CatchUp = state.m_LastBatchSize > 0 && ring_used > 2 * state.m_LastBatchSize;
		}
	}


	void USBSid::Write(unsigned char inSidReg, unsigned char inData, int inCycle)
	{
		if (!m_Active || inSidReg >= SID_REGISTER_COUNT)
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);

		const unsigned int cycle = inCycle < 0 ? 0 : static_cast<unsigned int>(inCycle);
		const uint64_t now = m_FrameBase + cycle;

		for (const Target& target : m_Targets)
			QueueWrite(target, inSidReg, inData, now, cycle);
	}


	void USBSid::EndFrame(unsigned int inCyclesInFrame)
	{
		if (!m_Active)
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);

		m_FrameBase += inCyclesInFrame;

		// Hand the frame to the driver in one batch per board: one lock, one thread wakeup
		for (size_t board = 0; board < m_BoardStates.size(); ++board)
		{
			BoardState& state = m_BoardStates[board];

			if (state.m_Pending.empty())
				continue;

			// The driver ring has no overflow guard, drop instead of overwriting unsent data
			if (m_Manager->RingFreeBytes(state.m_LogicalSID) < static_cast<int>(state.m_Pending.size()) + RING_LOW_WATER)
			{
				if (!state.m_Stalled)
					Logging::instance().Warning("USBSID-Pico: board %d stopped accepting writes, dropping writes", static_cast<int>(board));

				state.m_Stalled = true;
				state.m_Synced = false;
			}
			else
			{
				state.m_Stalled = false;
				state.m_LastBatchSize = state.m_Pending.size();
				m_Manager->WriteRingCycledN(state.m_LogicalSID, state.m_Pending.data(), static_cast<int>(state.m_Pending.size() / 4));
			}

			state.m_Pending.clear();
		}

		m_Manager->FlushAll();
	}


	void USBSid::Flush()
	{
		if (!m_Active)
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Manager->FlushAll();
	}

	//----------------------------------------------------------------------------------------------------------------

	bool USBSid::Open(bool inAllDetectedBoards)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (m_IsOpen)
			return true;

		// An empty serial opens the first board in bus and port order
		std::vector<std::string> serials = m_SelectedBoards;

		if (serials.empty() && inAllDetectedBoards)
			serials = m_DetectedBoards;

		if (serials.empty())
			serials.push_back(std::string());

		if (!m_Manager->OpenAll(serials, true, true))
		{
			Logging::instance().Warning("USBSID-Pico: no board could be opened");
			return false;
		}

		// Collect the SIDs to write to: the selected SIDs, else the first SID of every board, or all of them
		const auto& boards = m_Manager->Boards();
		const auto& logical_map = m_Manager->LogicalMap();

		m_Targets.clear();

		for (size_t i = 0; i < logical_map.size(); ++i)
		{
			const auto& slot = logical_map[i];

			for (const auto& selected : m_SelectedSIDs)
			{
				if (selected.first == boards[slot.board_index].serial && selected.second == slot.local_slot + 1)
					m_Targets.push_back({ static_cast<int>(i), slot.board_index, static_cast<unsigned char>(slot.local_slot * 0x20) });
			}
		}

		if (m_Targets.empty())
		{
			if (!m_SelectedSIDs.empty())
				Logging::instance().Warning("USBSID-Pico: none of the selected SIDs found, using the default SID of every board");

			int last_board = -1;

			for (size_t i = 0; i < logical_map.size(); ++i)
			{
				const auto& slot = logical_map[i];

				if (m_AllSIDs || slot.board_index != last_board)
					m_Targets.push_back({ static_cast<int>(i), slot.board_index, static_cast<unsigned char>(slot.local_slot * 0x20) });

				last_board = slot.board_index;
			}
		}

		if (m_Targets.empty())
		{
			Logging::instance().Warning("USBSID-Pico: opened boards have no SID configured");
			m_Manager->CloseAll();
			return false;
		}

		m_BoardStates.assign(m_Manager->BoardCount(), BoardState());

		for (const Target& target : m_Targets)
			m_BoardStates[target.m_Board].m_LogicalSID = target.m_LogicalSID;

		m_FrameBase = 0;
		m_IsOpen = true;

		for (const auto& board : m_Manager->Boards())
			Logging::instance().Info("USBSID-Pico: opened board %d [%s] with %d SID(s)", board.index, board.serial.c_str(), board.numsids);

		for (const Target& target : m_Targets)
			Logging::instance().Info("USBSID-Pico: writing to board %d SID %d", target.m_Board + 1, target.m_RegisterBase / 0x20 + 1);

		m_Manager->ResetAllRegistersAll();
		ApplyClockRate();

		return true;
	}


	void USBSid::Close()
	{
		if (!m_IsOpen)
			return;

		Silence();

		std::lock_guard<std::mutex> lock(m_Mutex);

		m_Manager->ResetAllRegistersAll();
		m_Manager->CloseAll();

		m_Targets.clear();
		m_BoardStates.clear();
		m_IsOpen = false;
	}


	void USBSid::ApplyClockRate()
	{
		const long clock_rate = m_PAL ? EMULATION_CYCLES_PER_SECOND_PAL : EMULATION_CYCLES_PER_SECOND_NTSC;

		// Force: the region can change more than once while the board stays open
		m_Manager->SetClockRateAll(clock_rate, true, true);
		m_LeadCycles = static_cast<unsigned int>((static_cast<uint64_t>(m_LeadTimeMs) * clock_rate) / 1000);

		if (m_LeadCycles == 0)
			m_LeadCycles = 1;

		for (BoardState& state : m_BoardStates)
			state.m_Synced = false;

		m_PaceRunning = false;
	}


	void USBSid::QueueWrite(const Target& inTarget, unsigned char inSidReg, unsigned char inData, uint64_t inNow, unsigned int inCycle)
	{
		BoardState& state = m_BoardStates[inTarget.m_Board];

		uint64_t wait;

		if (!state.m_Synced || inNow > state.m_Clock + MAX_WAIT_CYCLES)
		{
			// Board is idle: start a new timeline, the lead time absorbs feed jitter
			wait = static_cast<uint64_t>(m_LeadCycles) + inCycle;
			state.m_Clock = inNow;
			state.m_Synced = true;
			state.m_CatchUp = false;
		}
		else
		{
			// Aim the write at its cycle, the board adds its own overhead to the wait
			const uint64_t land_at = state.m_Clock + BOARD_WRITE_OVERHEAD;
			wait = inNow > land_at ? inNow - land_at : 0;

			const uint64_t busy = wait + BOARD_WRITE_OVERHEAD;
			state.m_Clock += busy > BOARD_WRITE_MIN ? busy : BOARD_WRITE_MIN;

			// Shorten a long wait to let a lagging board catch up, the timeline keeps its place
			if (state.m_CatchUp && wait > CATCH_UP_MIN_WAIT)
				wait -= wait >> 6;
		}

		if (wait > MAX_WAIT_CYCLES)
			wait = MAX_WAIT_CYCLES;

		state.m_Pending.push_back(static_cast<uint8_t>(inTarget.m_RegisterBase + inSidReg));
		state.m_Pending.push_back(inData);
		state.m_Pending.push_back(static_cast<uint8_t>(wait >> 8));
		state.m_Pending.push_back(static_cast<uint8_t>(wait & 0xff));
	}


	void USBSid::QueueSilence()
	{
		for (const Target& target : m_Targets)
		{
			if (m_Manager->RingFreeBytes(target.m_LogicalSID) < SID_REGISTER_COUNT * 4 + 16)
				continue;

			for (const unsigned char sid_register : SILENCE_ORDER)
				m_Manager->WriteRingCycled(target.m_LogicalSID, static_cast<uint8_t>(target.m_RegisterBase + sid_register), 0, 0);
		}

		for (BoardState& state : m_BoardStates)
		{
			state.m_Synced = false;
			state.m_Pending.clear();
		}
	}
}
