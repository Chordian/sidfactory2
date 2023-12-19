#if !defined(__EXECUTIONHANDLER_H__)
#define __EXECUTIONHANDLER_H__

#include "foundation/sound/audiostream.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "runtime/editor/driver/driver_utils.h"

#define ASID_NUM_REGS 28

class RtMidiOut;

namespace Utility
{
	class ConfigFile;
}

namespace Foundation
{
	class IMutex;
	class IPlatform;
}

namespace Emulation
{
	class CPUmos6510;
	class CPUMemory;
	class SIDProxy;
	class FlightRecorder;

	class ExecutionHandler : public Foundation::IAudioStreamFeeder
	{
	public:
		struct SIDRegistersBuffer
		{
			unsigned char m_Buffer[0x200];
		};
		
		ExecutionHandler(
			CPUmos6510* pCPU,
			CPUMemory* pMemory,
			SIDProxy* pSIDProxy,
			FlightRecorder* inFlightRecorder);
		~ExecutionHandler();

		// IAudioStreamFeeder

		virtual void Start();
		virtual void Stop();

		virtual bool IsStarted() const;

		virtual unsigned int GetFeedCount() const;
		virtual unsigned int GetBytesFed() const;

		virtual void PreFeedPCM(void* inBuffer, unsigned int inByteCount);
		virtual void FeedPCM(void* inBuffer, unsigned int inByteCount);

		// Lock and unlock
		void Lock();
		void Unlock();

		// Settings
		void SetPAL(const bool inPALMode);

		// Error
		bool IsInErrorState() const;
		std::string GetErrorMessage() const;
		void ClearErrorState();

		// Emulation update
		void SetEnableUpdate(bool inEnableUpdate);
		void SetFastForward(unsigned int inFastForwardUpdateCount);

		void QueueInit(unsigned char inInitArgument);
		void QueueInit(unsigned char inInitArgument, const std::function<void(CPUMemory*)>& inPostInitCallback);
		void QueueStop();
		void QueueMuteChannel(unsigned char inChannel, const std::function<void(CPUMemory*)>& inMuteCallback);
		void QueueClearAllMuteState(const std::function<void(CPUMemory*)>& inClearMuteStateCallback);

		void SetInitVector(unsigned short inVector);
		void SetStopVector(unsigned short inVector);
		void SetUpdateVector(unsigned short inVector);
		void SetPostUpdateCallback(const std::function<void(CPUMemory*)>& inPostUpdateCallback);

		// Cycles
		unsigned int GetCPUCyclesSpendLastFrame() const { return m_CPUCyclesSpend; }
		unsigned int GetCPUFrameUpdateCount() const { return m_CPUFrameCounter; }

		// Frame
		unsigned int GetFrameCounter() const { return m_CPUFrameCounter; }

		// Flight recorder
		FlightRecorder* GetFlightRecorder() const { return m_SIDRegisterFlightRecorder; }

		// SID registers buffer
		SIDRegistersBuffer GetSIDRegistersBufferAfterLastDriverUpdate() const { return m_SIDRegisterLastDriverUpdate; }

		// Write output to file
		void StartWriteOutputToFile(const std::string& inFilename);
		void StopWriteOutputToFile();
		bool IsWritingOutputToFile() const;

		void SendASIDWriteOrder(std::vector<Editor::SIDWriteInformation> SIDWriteInfoList);

	private:
		enum class ActionType : int
		{
			Init,
			Stop,
			Update,
			ApplyMuteState,
			ClearMuteAllState
		};

		struct Action
		{
			ActionType m_ActionType;
			unsigned char m_ActionArgument;
			std::function<void(CPUMemory*)> m_PostActionCallback;
		};

		const unsigned short GetAddressFromActionType(ActionType inActionType) const;

		void SimulateSID(int inDeltaCycles);
		unsigned char GetASIDposFromSIDreg(unsigned char ucSIDReg);
		void ASIDWrite(unsigned char ucSidReg, unsigned char ucData);
		void ASIDSend();
		void CaptureNewFrame();

		// Audio stream feeding

		unsigned int m_FeedCount;
		unsigned int m_BytesFedCount;

		unsigned int m_CurrentCycle; // Current cycle being processed
		unsigned int m_CyclesPerFrame; // Number of cycles per frame
		unsigned int m_CPUCyclesSpend; // Cycles spend on code during the last update (frame)

		unsigned int m_CPUFrameCounter;

		unsigned int m_SampleBufferReadCursor;
		unsigned int m_SampleBufferWriteCursor;

		bool m_IsStarted;

		// Error state
		bool m_ErrorState;
		std::string m_ErrorMessage;

		// Action
		std::vector<Action> m_ActionQueue;

		// Update
		bool m_UpdateEnabled;
		unsigned int m_FastForwardUpdateCount;
		std::function<void(CPUMemory*)> m_PostUpdateCallback;

		// Driver vectors
		unsigned short m_InitVector;
		unsigned short m_StopVector;
		unsigned short m_UpdateVector;

		// SID and CPU
		SIDProxy* m_SIDProxy;
		CPUmos6510* m_CPU;
		CPUMemory* m_Memory;

		std::shared_ptr<Foundation::IMutex> m_Mutex;

		// Flight recorder
		FlightRecorder* m_SIDRegisterFlightRecorder;

		// SID Registers last update
		SIDRegistersBuffer m_SIDRegisterLastDriverUpdate;

		// Audio output
		unsigned int m_SampleBufferSize;
		short* m_SampleBuffer;
		float m_OutputGain;

		// ASID
		RtMidiOut* m_pRtMidiOut;
		unsigned char m_aucAsidRegisterBuffer[ASID_NUM_REGS];
		unsigned char m_aucAsidRegisterUpdated[ASID_NUM_REGS];
	};
}

#endif //__EXECUTIONHANDLER_H__
