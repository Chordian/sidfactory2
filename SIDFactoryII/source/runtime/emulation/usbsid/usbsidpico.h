#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

class USBSID_Manager;

namespace Emulation
{
	class USBSid
	{
	public:
		struct SIDInfo
		{
			std::string m_BoardSerial;
			int m_BoardNumber;		// 1 based, in open order
			int m_SIDNumber;		// 1 based, as numbered by the board
			int m_Type;				// 0 unknown, 1 none, 2 MOS8580, 3 MOS6581, 4 FMopl
			bool m_Selected;
		};

		USBSid();
		~USBSid();

		USBSid(const USBSid&) = delete;
		USBSid& operator=(const USBSid&) = delete;

		// Board detection and selection
		unsigned int DetectBoards();
		const std::vector<std::string>& GetDetectedBoards() const { return m_DetectedBoards; }
		bool IsBoardSelectionRequired() const;
		void SelectBoards(const std::vector<std::string>& inSerials);

		// SID selection: list every SID on the boards in use, pick the ones that receive the writes
		std::vector<SIDInfo> QuerySIDs();
		void SelectSIDs(const std::vector<SIDInfo>& inSIDs);

		// Output control
		bool SetActive(bool inActive);
		bool IsActive() const { return m_Active; }
		bool IsOpen() const { return m_IsOpen; }
		void SetPAL(bool inPAL);

		// Number of SIDs the tune plays on. Target n receives tune SID n modulo this count
		void SetSIDCount(unsigned int inSIDCount);

		// Restart the write timeline on the first following write
		void Resync();
		// Zero all SID registers
		void Silence();

		// Number of frames to feed to keep up with the wall clock
		unsigned int FramesDue(unsigned int inCyclesInFrame);

		// Frame feed, cycles count from the start of the frame
		void BeginFrame();
		void Write(unsigned int inSID, unsigned char inSidReg, unsigned char inData, int inCycle);
		void EndFrame(unsigned int inCyclesInFrame);
		void Flush();

	private:
		struct Target
		{
			int m_LogicalSID;
			int m_Board;
			unsigned char m_RegisterBase;
		};

		struct BoardState
		{
			uint64_t m_Clock = 0;		// Emulated cycle at which the board is free again
			bool m_Synced = false;		// Write timeline is running
			bool m_Stalled = false;		// Ring buffer stopped draining
			bool m_CatchUp = false;		// Board lags behind the timeline
			int m_LogicalSID = 0;		// Any SID on the board, routes the write batch
			size_t m_LastBatchSize = 0;	// Bytes queued by the previous frame
			std::vector<uint8_t> m_Pending;	// Writes of the current frame: reg, value, wait hi, wait lo
		};

		bool Open(bool inAllDetectedBoards = false);
		void Close();
		void ApplyClockRate();
		void BuildTargets();
		void QueueWrite(const Target& inTarget, unsigned char inSidReg, unsigned char inData, uint64_t inNow, unsigned int inCycle);
		void QueueSilence();

		std::unique_ptr<USBSID_Manager> m_Manager;
		std::mutex m_Mutex;

		std::vector<std::string> m_DetectedBoards;
		std::vector<std::string> m_SelectedBoards;
		bool m_BoardsSelected;
		std::vector<std::pair<std::string, int>> m_SelectedSIDs;	// Board serial, SID number

		std::vector<Target> m_Targets;
		std::vector<BoardState> m_BoardStates;

		std::atomic<bool> m_Active;
		bool m_IsOpen;
		bool m_PAL;
		bool m_AllSIDs;
		unsigned int m_SIDCount;

		unsigned int m_LeadTimeMs;
		unsigned int m_LeadCycles;

		uint64_t m_FrameBase;		// Emulated cycle at the start of the current frame
		std::chrono::steady_clock::time_point m_LastFrameTime;

		// Wall clock pacing
		bool m_PaceRunning;
		std::chrono::steady_clock::time_point m_PaceStart;
		double m_PacedCycles;		// Emulated cycles handed out since m_PaceStart
	};
}
