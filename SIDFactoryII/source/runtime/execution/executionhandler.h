#if !defined(__EXECUTIONHANDLER_H__)
#define __EXECUTIONHANDLER_H__

#include "foundation/sound/audiostream.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "runtime/editor/driver/driver_utils.h"
#include "utils/sidpanning.h"

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
	class ASid;
	class USBSid;
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
			ASid* inASID,
			USBSid* inUSBSID,
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
		virtual void SetChannelCount(unsigned int inChannelCount);

		// Lock and unlock
		void Lock();
		void Unlock();

		// Settings
		void SetPAL(const bool inPALMode);

		// Number of SID chips the loaded driver plays on, SID n sits at $d400 + n * $20
		static const unsigned int MaxSIDCount = 4;
		void SetSIDCount(unsigned int inSIDCount);
		unsigned int GetSIDCount() const { return m_SIDCount; }

		// Stereo position of the SIDs in the reSID output. Multi SID: SID v5 layout and mode,
		// single SID: inSingleSIDPan
		void SetPanning(Utility::SIDPanLayout inLayout, Utility::SIDPanMode inMode, Utility::SIDPan inSingleSIDPan);
		Utility::SIDPan GetSIDPan(unsigned int inSIDIndex) const;

		// 2 when a SID is panned left or right, the audio stream is reopened to match
		unsigned int GetWantedChannelCount() const;

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

		// SID Write order info and environment, for ASID usage
		void TellSIDWriteOrderInfo(std::vector<Editor::SIDWriteInformation> SIDWriteInfoList);
		void TellSIDEnvironment();

		enum class OutputDevice: int
		{
			RESID,
			ASID,
			USBSID
		};

		void SetOutputDevice(const OutputDevice device);
		const OutputDevice GetOutputDevice() const;

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

		// One write to a SID register within a frame
		struct SIDWrite
		{
			int m_Cycle;
			unsigned short m_Address;
			unsigned char m_Value;
		};

		// SID 2 and up of a multi SID driver. Their frame is rendered on other threads, the
		// audio callback has no time to clock more than one SID
		struct ExtraSID
		{
			std::unique_ptr<SIDProxy> m_SID;
			std::vector<SIDWrite> m_Writes;		// Writes of the current frame, m_Address holds the register
			std::vector<short> m_Samples;		// Output of the current frame
			int m_SampleCount = 0;
			std::thread m_Thread;
		};

		void SimulateSID(int inDeltaCycles);
		void SyncExtraSIDs();

		void StartExtraSIDThreads();
		void StopExtraSIDThreads();
		void ExtraSIDThread();
		void RenderClaimedExtraSIDs(int inCyclesInFrame);
		void RenderExtraSID(ExtraSID& inExtraSID, int inCyclesInFrame);
		void MixSIDs();
		void UpdateSIDPanning();

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

		// SID 2 and up of a multi SID driver, mixed into the output of the first SID
		unsigned int m_SIDCount;
		Utility::SIDPanLayout m_PanLayout;
		Utility::SIDPanMode m_PanMode;
		Utility::SIDPan m_SingleSIDPan;
		Utility::SIDPan m_SIDPan[MaxSIDCount];
		std::vector<std::unique_ptr<ExtraSID>> m_ExtraSIDs;
		std::vector<SIDWrite> m_FrameWrites;

		// Hand over of a frame to the threads of the extra SIDs
		std::mutex m_RenderMutex;
		std::condition_variable m_RenderStart;
		std::condition_variable m_RenderDone;
		unsigned int m_RenderGeneration;	// Counts the frames handed over
		unsigned int m_RenderPending;		// Extra SIDs not done with the current frame
		std::atomic<unsigned int> m_RenderNext;	// First extra SID of the current frame no thread has taken
		int m_RenderCyclesInFrame;
		bool m_RenderQuit;
		CPUmos6510* m_CPU;
		CPUMemory* m_Memory;
		ASid* m_ASID;
		USBSid* m_USBSID;

		std::shared_ptr<Foundation::IMutex> m_Mutex;

		// Flight recorder
		FlightRecorder* m_SIDRegisterFlightRecorder;

		// SID Registers last update
		SIDRegistersBuffer m_SIDRegisterLastDriverUpdate;

		// Audio output
		unsigned int m_SampleBufferSize;
		short* m_SampleBuffer;			// Mono output, or the left side of stereo output
		short* m_SampleBufferRight;		// Right side of stereo output
		unsigned int m_OutputChannelCount;
		float m_OutputGain;
		OutputDevice m_OutputDevice;
		bool m_SkipSIDSimulation;
	};
}

#endif //__EXECUTIONHANDLER_H__
