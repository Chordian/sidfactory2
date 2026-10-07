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

		// Interval of the attached board check while a lost output waits for its boards
		const std::chrono::milliseconds RECONNECT_POLL(1000);

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
		, m_Reconnecting(false)
		, m_ReconnectPoll(std::chrono::steady_clock::now())
		, m_Active(false)
		, m_IsOpen(false)
		, m_PAL(true)
		, m_AllSIDs(false)
		, m_SIDCount(1)
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

		// Selected SIDs name their boards
		if (!m_BoardsSelected)
		{
			for (const auto& selected : m_SelectedSIDs)
			{
				bool known_board = false;

				for (const std::string& serial : m_SelectedBoards)
					known_board |= serial == selected.first;

				if (!known_board)
					m_SelectedBoards.push_back(selected.first);
			}

			m_BoardsSelected = !m_SelectedBoards.empty();
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
		CloseIfLost();

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
		m_Reconnecting = false;

		if (was_active)
			SetActive(true);
	}


	std::vector<USBSid::SIDInfo> USBSid::QuerySIDs(bool inAllBoards)
	{
		CloseIfLost();

		if (inAllBoards)
		{
			std::vector<SIDInfo> sids;

			// Mark the chosen SIDs, else the SIDs in use
			std::vector<std::pair<std::string, int>> marks = m_SelectedSIDs;

			if (marks.empty() && m_IsOpen)
			{
				std::lock_guard<std::mutex> lock(m_Mutex);

				const auto& boards = m_Manager->Boards();

				for (const Target& target : m_Targets)
					marks.push_back({ boards[target.m_Board].serial, target.m_RegisterBase / 0x20 + 1 });
			}

			// The socket configuration is readable on an open board only: open every attached board
			const bool was_active = m_Active;

			m_Active = false;
			Close();
			DetectBoards();

			if (!m_DetectedBoards.empty() && OpenBoards(m_DetectedBoards))
			{
				{
					std::lock_guard<std::mutex> lock(m_Mutex);

					const auto& boards = m_Manager->Boards();
					const auto& logical_map = m_Manager->LogicalMap();

					for (size_t i = 0; i < logical_map.size(); ++i)
					{
						const auto& slot = logical_map[i];
						const std::string& serial = boards[slot.board_index].serial;
						bool selected = false;

						if (!marks.empty())
						{
							for (const auto& mark : marks)
								selected |= mark.first == serial && mark.second == slot.local_slot + 1;
						}
						else
						{
							// Default SIDs, limited to the chosen boards
							bool board_in_use = !m_BoardsSelected;

							for (const std::string& selected_serial : m_SelectedBoards)
								board_in_use |= selected_serial == serial;

							for (const Target& target : m_Targets)
								selected |= board_in_use && target.m_LogicalSID == static_cast<int>(i);
						}

						sids.push_back({ serial, slot.board_index + 1, slot.local_slot + 1, slot.sid_type, selected });
					}
				}

				Close();
			}

			if (was_active)
				SetActive(true);

			return sids;
		}

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
		m_Reconnecting = false;

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

	std::string USBSid::DescribeSID(const SIDInfo& inSID)
	{
		static const char* sid_type_names[] = { "unknown", "none", "8580", "6581", "FMopl" };

		const std::string type_name = inSID.m_Type >= 0 && inSID.m_Type <= 4 ? sid_type_names[inSID.m_Type] : "unknown";
		const std::string serial = inSID.m_BoardSerial.empty() ? "no serial" : inSID.m_BoardSerial;

		return "Board " + std::to_string(inSID.m_BoardNumber) + " [" + serial + "] SID " + std::to_string(inSID.m_SIDNumber) + " (" + type_name + ")";
	}

	//----------------------------------------------------------------------------------------------------------------
	// Lost boards
	//----------------------------------------------------------------------------------------------------------------

	USBSid::Event USBSid::Update()
	{
		if (m_IsOpen && m_Manager->AnyBoardLost())
		{
			const bool was_active = m_Active;

			m_ReconnectSerials.clear();

			for (const auto& board : m_Manager->Boards())
			{
				if (m_Manager->BoardLost(board.index))
					Logging::instance().Warning("USBSID-Pico: board %d [%s] lost", board.index + 1, board.serial.c_str());

				m_ReconnectSerials.push_back(board.serial);
			}

			m_Active = false;
			Close();

			// An output not in use closes without a trace, the next activation opens it again
			if (!was_active)
				return Event::None;

			m_Reconnecting = true;
			m_ReconnectPoll = std::chrono::steady_clock::now();

			return Event::Lost;
		}

		if (!m_Reconnecting)
			return Event::None;

		// Opened by other means in the meantime
		if (m_IsOpen)
		{
			m_Reconnecting = false;
			return Event::None;
		}

		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		if (now - m_ReconnectPoll < RECONNECT_POLL)
			return Event::None;

		m_ReconnectPoll = now;

		if (!AreBoardsAttached(m_ReconnectSerials))
			return Event::None;

		DetectBoards();

		// A board still starting up fails to open, try again on the next poll
		if (!OpenBoards(m_ReconnectSerials))
			return Event::None;

		m_Reconnecting = false;
		Logging::instance().Info("USBSID-Pico: reconnected");

		return Event::Reconnected;
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
			CloseIfLost();

			if (!m_IsOpen && !Open())
				return false;

			Resync();
			m_Reconnecting = false;
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


	void USBSid::SetSIDCount(unsigned int inSIDCount)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		const unsigned int sid_count = inSIDCount < 1 ? 1 : inSIDCount;

		if (sid_count == m_SIDCount)
			return;

		m_SIDCount = sid_count;

		// The default choice of SIDs depends on the count
		if (m_IsOpen)
		{
			QueueSilence();
			m_Manager->FlushAll();
			BuildTargets();
		}
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


	void USBSid::Write(unsigned int inSID, unsigned char inSidReg, unsigned char inData, int inCycle)
	{
		if (!m_Active || inSidReg >= SID_REGISTER_COUNT)
			return;

		std::lock_guard<std::mutex> lock(m_Mutex);

		const unsigned int cycle = inCycle < 0 ? 0 : static_cast<unsigned int>(inCycle);
		const uint64_t now = m_FrameBase + cycle;

		// Target n plays tune SID n modulo the SID count: more targets than tune SIDs mirror
		for (size_t i = 0; i < m_Targets.size(); ++i)
		{
			if (i % m_SIDCount == inSID)
				QueueWrite(m_Targets[i], inSidReg, inData, now, cycle);
		}
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
		std::vector<std::string> serials = m_SelectedBoards;

		if (serials.empty() && inAllDetectedBoards)
			serials = m_DetectedBoards;

		return OpenBoards(serials);
	}


	bool USBSid::OpenBoards(const std::vector<std::string>& inSerials)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (m_IsOpen)
			return true;

		// An empty serial opens the first board in bus and port order
		std::vector<std::string> serials = inSerials;

		if (serials.empty())
			serials.push_back(std::string());

		if (!m_Manager->OpenAll(serials, true, true))
		{
			Logging::instance().Warning("USBSID-Pico: no board could be opened");
			return false;
		}

		BuildTargets();

		if (m_Targets.empty())
		{
			Logging::instance().Warning("USBSID-Pico: opened boards have no SID configured");
			m_Manager->CloseAll();
			return false;
		}

		m_FrameBase = 0;
		m_IsOpen = true;

		for (const auto& board : m_Manager->Boards())
			Logging::instance().Info("USBSID-Pico: opened board %d [%s] with %d SID(s)", board.index, board.serial.c_str(), board.numsids);

		m_Manager->ResetAllRegistersAll();
		ApplyClockRate();

		return true;
	}


	void USBSid::BuildTargets()
	{
		const auto& boards = m_Manager->Boards();
		const auto& logical_map = m_Manager->LogicalMap();

		m_Targets.clear();

		// The selected SIDs, in board order
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
				Logging::instance().Warning("USBSID-Pico: none of the selected SIDs found, using the default SIDs");

			// Default: every SID with AllSIDs, the first SIDs in board order for a multi SID tune,
			// else the first SID of every board
			int last_board = -1;

			for (size_t i = 0; i < logical_map.size(); ++i)
			{
				const auto& slot = logical_map[i];
				const bool use = m_AllSIDs || (m_SIDCount > 1 ? m_Targets.size() < m_SIDCount : slot.board_index != last_board);

				if (use)
					m_Targets.push_back({ static_cast<int>(i), slot.board_index, static_cast<unsigned char>(slot.local_slot * 0x20) });

				last_board = slot.board_index;
			}
		}

		m_BoardStates.assign(m_Manager->BoardCount(), BoardState());

		for (const Target& target : m_Targets)
			m_BoardStates[target.m_Board].m_LogicalSID = target.m_LogicalSID;

		for (size_t i = 0; i < m_Targets.size(); ++i)
			Logging::instance().Info("USBSID-Pico: tune SID %d on board %d SID %d", static_cast<int>(i % m_SIDCount) + 1, m_Targets[i].m_Board + 1, m_Targets[i].m_RegisterBase / 0x20 + 1);

		if (m_Targets.size() < m_SIDCount)
			Logging::instance().Warning("USBSID-Pico: the tune uses %d SIDs, %d available, the remaining SIDs stay silent", static_cast<int>(m_SIDCount), static_cast<int>(m_Targets.size()));
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


	void USBSid::CloseIfLost()
	{
		// An active output is closed by Update(), which reports the loss
		if (m_IsOpen && !m_Active && m_Manager->AnyBoardLost())
		{
			Logging::instance().Warning("USBSID-Pico: board lost while not in use, closing");
			Close();
		}
	}


	bool USBSid::AreBoardsAttached(const std::vector<std::string>& inSerials) const
	{
		const std::vector<USBSID_NS::USBSID_DeviceInfo> devices = USBSID_Manager::Enumerate();

		for (const std::string& serial : inSerials)
		{
			bool attached = false;

			// An empty serial stands for the first board found
			for (const auto& device : devices)
				attached |= serial.empty() || device.serial == serial;

			if (!attached)
				return false;
		}

		return !devices.empty();
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
