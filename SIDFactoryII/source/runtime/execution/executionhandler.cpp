#include "executionhandler.h"

#include "libraries/residfp/siddefs-fp.h"
#include "runtime/emulation/cpuframecapture.h"
#include "runtime/emulation/cpumos6510.h"
#include "runtime/emulation/sid/sidproxy.h"

#include "runtime/environmentdefines.h"
#include "runtime/execution/flightrecorder.h"

#include "foundation/base/assert.h"
#include "foundation/platform/imutex.h"
#include "foundation/platform/iplatform.h"
#include "runtime/emulation/asid/asid.h"
#include "runtime/emulation/usbsid/usbsidpico.h"
#include "libraries/rtmidi/RtMidi.h"

#include "utils/configfile.h"
#include "utils/logging.h"
#include "utils/global.h"

#include <array>
#include <algorithm>
#include <iostream>


using namespace Foundation;
using namespace Utility;

namespace Emulation
{

	// clamp multiplied samples to these limits
	const float sampleCeiling = 32767.0f;
	const float sampleFloor = -32767.0f;

	ExecutionHandler::ExecutionHandler(
		CPUmos6510* inCPU,
		CPUMemory* pMemory,
		SIDProxy* pSIDProxy,
		ASid* inASID,
		USBSid* inUSBSID,
		FlightRecorder* inFlightRecorder)
		: m_CPU(inCPU)
		, m_Memory(pMemory)
		, m_SIDProxy(pSIDProxy)
		, m_ASID(inASID)
		, m_USBSID(inUSBSID)
		, m_SIDRegisterFlightRecorder(inFlightRecorder)
		, m_IsStarted(false)
		, m_FeedCount(0)
		, m_BytesFedCount(0)
		, m_SampleBufferReadCursor(0)
		, m_SampleBufferWriteCursor(0)
		, m_CPUFrameCounter(0)
		, m_UpdateEnabled(false)
		, m_ErrorState(false)
		, m_OutputDevice(OutputDevice::RESID)
		, m_SkipSIDSimulation(false)
		, m_SIDCount(1)
		, m_RenderGeneration(0)
		, m_RenderPending(0)
		, m_RenderCyclesInFrame(0)
		, m_RenderQuit(false)
	{
		m_CyclesPerFrame = EMULATION_CYCLES_PER_FRAME_PAL;

		// Create a sample buffer. The sample frequency is used for determining the size, which is probably 50 times the size required.
		m_SampleBufferSize = (static_cast<unsigned int>(pSIDProxy->GetSampleFrequency()) << 8);
		m_SampleBuffer = new short[m_SampleBufferSize];
		m_Mutex = Global::instance().GetPlatform().CreateMutex();
		m_OutputGain = GetSingleConfigurationValue<Utility::Config::ConfigValueFloat>(Global::instance().GetConfig(), "Sound.Output.Gain", -1.0f);

		Logging::instance().Info("Sound.Output.Gain = %f", m_OutputGain);

		// Set default action vector
		m_InitVector = 0x1000;
		m_StopVector = 0x1003;
		m_UpdateVector = 0x1006;

		// Clear SID registers after last driver update
		memset(m_SIDRegisterLastDriverUpdate.m_Buffer, 0, sizeof(m_SIDRegisterLastDriverUpdate.m_Buffer));
	}

	ExecutionHandler::~ExecutionHandler()
	{
		StopExtraSIDThreads();

		m_Mutex = nullptr;

		if (m_SampleBuffer != nullptr)
			delete[] m_SampleBuffer;
	}

	//----------------------------------------------------------------------------------------------------------------
	// IAudioStreamFeeder
	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::Start()
	{
		if (!m_IsStarted)
		{
			m_FeedCount = 0;
			m_BytesFedCount = 0;
			m_CPUCyclesSpend = 0;
			m_CPUFrameCounter = 0;

			if (m_SIDProxy != nullptr)
				m_SIDProxy->Reset();

			for (auto& extra_sid : m_ExtraSIDs)
				extra_sid->m_SID->Reset();

			if (m_SIDRegisterFlightRecorder != nullptr)
			{
				m_SIDRegisterFlightRecorder->Lock();
				m_SIDRegisterFlightRecorder->Reset();
				m_SIDRegisterFlightRecorder->Unlock();
			}

			if (m_USBSID != nullptr)
				m_USBSID->Resync();

			m_IsStarted = true;
		}
	}

	void ExecutionHandler::Stop()
	{
		if (m_IsStarted)
		{
			m_SampleBufferReadCursor = 0;
			m_SampleBufferWriteCursor = 0;

			// No frames are fed while stopped, do not leave notes hanging on the board
			if (m_USBSID != nullptr && m_OutputDevice == OutputDevice::USBSID)
				m_USBSID->Silence();

			m_IsStarted = false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------

	bool ExecutionHandler::IsStarted() const
	{
		return m_IsStarted;
	}

	//----------------------------------------------------------------------------------------------------------------

	unsigned int ExecutionHandler::GetBytesFed() const
	{
		return m_BytesFedCount;
	}

	unsigned int ExecutionHandler::GetFeedCount() const
	{
		return m_FeedCount;
	}

	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::PreFeedPCM(void* inBuffer, unsigned int inByteCount)
	{
		FeedPCM(inBuffer, inByteCount);
	}

	void ExecutionHandler::SetOutputDevice(const OutputDevice device)
	{
		if (m_OutputDevice == device)
			return;

		// if MIDI port is not open, do not switch to ASID
		if (device == OutputDevice::ASID && (m_ASID == nullptr || !m_ASID->isPortOpen()))
			return;

		// if no USBSID-Pico board can be opened, do not switch to USBSID
		if (device == OutputDevice::USBSID && (m_USBSID == nullptr || !m_USBSID->SetActive(true)))
			return;

		if (device != OutputDevice::USBSID && m_USBSID != nullptr)
			m_USBSID->SetActive(false);

		m_OutputDevice = device;

		// mute/unmute ASID depending on its selection
		if (m_ASID != nullptr && m_ASID->isPortOpen())
			m_ASID->SetMuted(m_OutputDevice != OutputDevice::ASID);

		Utility::Logging::instance().Info("OutputDevice set to %s",
			m_OutputDevice == OutputDevice::ASID ? "ASID" : (m_OutputDevice == OutputDevice::USBSID ? "USBSID" : "RESID"));
	}

	const ExecutionHandler::OutputDevice ExecutionHandler::GetOutputDevice() const
	{
		return m_OutputDevice;
	}

	void ExecutionHandler::FeedPCM(void* inBuffer, unsigned int inByteCount)
	{
		m_FeedCount++;
		m_BytesFedCount += inByteCount;

		if (!m_IsStarted)
		{
			memset(inBuffer, 0, inByteCount);
		}
		else if (m_OutputDevice == OutputDevice::USBSID && m_USBSID != nullptr)
		{
			// The board makes the sound: feed frames by the wall clock, not by sample demand
			memset(inBuffer, 0, inByteCount);

			const unsigned int frames_due = m_USBSID->FramesDue(m_CyclesPerFrame);

			for (unsigned int i = 0; i < frames_due; ++i)
				CaptureNewFrame();

			// Send writes the frame flush left in the driver buffer
			m_USBSID->Flush();
		}
		else
		{
			unsigned int uiRemainingSamples = (inByteCount >> 1);

			short* pSource = static_cast<short*>(m_SampleBuffer);
			short* pTarget = static_cast<short*>(inBuffer);

			while (uiRemainingSamples > 0)
			{
				FOUNDATION_ASSERT(m_SampleBufferReadCursor <= m_SampleBufferWriteCursor);

				if (m_SampleBufferReadCursor >= m_SampleBufferWriteCursor)
				{
					// Capture a single frame of audio
					CaptureNewFrame();
				}

				const unsigned int uiRemainingSourceSamples = m_SampleBufferWriteCursor - m_SampleBufferReadCursor;
				const unsigned int uiSamplesToCopy = uiRemainingSamples > uiRemainingSourceSamples ? uiRemainingSourceSamples : uiRemainingSamples;

				for (unsigned int i = 0; i < uiSamplesToCopy; ++i)
				{
					if (m_OutputDevice == ExecutionHandler::OutputDevice::RESID)
					{
						const float fSample = static_cast<float>(pSource[i + m_SampleBufferReadCursor]) * m_OutputGain;
						const float fClampedSample = fmin(sampleCeiling, fmax(fSample, sampleFloor));
						pTarget[i] = static_cast<short>(fClampedSample);
					}
					else
					{
						pTarget[i] = 0;
					}
				}

				// Forward the read cursor
				m_SampleBufferReadCursor += uiSamplesToCopy;

				// Forward the target pointer
				pTarget += uiSamplesToCopy;

				// Decrement the remaining number of samples
				uiRemainingSamples -= uiSamplesToCopy;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------
	// Lock and unlock
	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::Lock()
	{
		if (m_Mutex != nullptr)
			m_Mutex->Lock();
	}

	void ExecutionHandler::Unlock()
	{
		if (m_Mutex != nullptr)
			m_Mutex->Unlock();
	}

	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::SetPAL(const bool inPALMode)
	{
		m_CyclesPerFrame = inPALMode ? EMULATION_CYCLES_PER_FRAME_PAL : EMULATION_CYCLES_PER_FRAME_NTSC;

		if (m_USBSID != nullptr)
			m_USBSID->SetPAL(inPALMode);
	}


	void ExecutionHandler::SetSIDCount(unsigned int inSIDCount)
	{
		const unsigned int sid_count = inSIDCount < 1 ? 1 : (inSIDCount > MaxSIDCount ? MaxSIDCount : inSIDCount);

		Lock();

		if (sid_count != m_SIDCount)
		{
			StopExtraSIDThreads();

			m_SIDCount = sid_count;
			m_ExtraSIDs.clear();

			// Same configuration as the first SID, kept in step by SyncExtraSIDs()
			for (unsigned int i = 1; i < m_SIDCount; ++i)
			{
				m_ExtraSIDs.push_back(std::make_unique<ExtraSID>());

				ExtraSID& extra_sid = *m_ExtraSIDs.back();

				extra_sid.m_SID = std::make_unique<SIDProxy>(m_SIDProxy->GetConfiguration());
				extra_sid.m_SID->Reset();

				// Room for more than one frame at the lowest frame rate
				extra_sid.m_Samples.resize(static_cast<size_t>(m_SIDProxy->GetSampleFrequency()) / 8);
			}

			StartExtraSIDThreads();
		}

		Unlock();

		if (m_USBSID != nullptr)
			m_USBSID->SetSIDCount(sid_count);
	}


	void ExecutionHandler::SyncExtraSIDs()
	{
		for (auto& extra_sid : m_ExtraSIDs)
		{
			SIDProxy& sid = *extra_sid->m_SID;

			if (sid.GetModel() != m_SIDProxy->GetModel() || sid.GetEnvironment() != m_SIDProxy->GetEnvironment())
			{
				sid.SetConfiguration(m_SIDProxy->GetConfiguration());
				sid.ApplySettings();
			}
		}
	}


	//----------------------------------------------------------------------------------------------------------------
	// Extra SID threads
	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::StartExtraSIDThreads()
	{
		m_RenderQuit = false;
		m_RenderPending = 0;

		for (unsigned int i = 0; i < m_ExtraSIDs.size(); ++i)
			m_ExtraSIDs[i]->m_Thread = std::thread(&ExecutionHandler::ExtraSIDThread, this, i);
	}


	void ExecutionHandler::StopExtraSIDThreads()
	{
		{
			std::lock_guard<std::mutex> lock(m_RenderMutex);
			m_RenderQuit = true;
		}

		m_RenderStart.notify_all();

		for (auto& extra_sid : m_ExtraSIDs)
		{
			if (extra_sid->m_Thread.joinable())
				extra_sid->m_Thread.join();
		}
	}


	void ExecutionHandler::ExtraSIDThread(unsigned int inIndex)
	{
		unsigned int generation = 0;

		{
			std::lock_guard<std::mutex> lock(m_RenderMutex);
			generation = m_RenderGeneration;
		}

		for (;;)
		{
			int cycles_in_frame = 0;

			{
				std::unique_lock<std::mutex> lock(m_RenderMutex);
				m_RenderStart.wait(lock, [&]() { return m_RenderQuit || m_RenderGeneration != generation; });

				if (m_RenderQuit)
					return;

				generation = m_RenderGeneration;
				cycles_in_frame = m_RenderCyclesInFrame;
			}

			RenderExtraSID(*m_ExtraSIDs[inIndex], cycles_in_frame);

			{
				std::lock_guard<std::mutex> lock(m_RenderMutex);
				--m_RenderPending;
			}

			m_RenderDone.notify_one();
		}
	}


	void ExecutionHandler::RenderExtraSID(ExtraSID& inExtraSID, int inCyclesInFrame)
	{
		short* samples = inExtraSID.m_Samples.data();
		const int sample_capacity = static_cast<int>(inExtraSID.m_Samples.size());

		int cycle = 0;
		int sample_count = 0;

		auto clock_to = [&](int inCycle)
		{
			int delta_cycles = inCycle - cycle;

			// A clock of 1 cycle yields 1 sample at most, stop before the buffer is full
			if (delta_cycles > 0 && sample_count + delta_cycles / 4 + 1 < sample_capacity)
				sample_count += inExtraSID.m_SID->Clock(delta_cycles, samples + sample_count, sample_capacity - sample_count);

			if (inCycle > cycle)
				cycle = inCycle;
		};

		for (const SIDWrite& write : inExtraSID.m_Writes)
		{
			clock_to(write.m_Cycle);
			inExtraSID.m_SID->Write(static_cast<unsigned char>(write.m_Address), write.m_Value);
		}

		clock_to(inCyclesInFrame);

		inExtraSID.m_SampleCount = sample_count;
	}


	void ExecutionHandler::MixExtraSIDs()
	{
		short* sample_buffer = static_cast<short*>(m_SampleBuffer);
		const int sample_count = static_cast<int>(m_SampleBufferWriteCursor);

		for (auto& extra_sid : m_ExtraSIDs)
		{
			const short* extra_samples = extra_sid->m_Samples.data();
			const int extra_count = extra_sid->m_SampleCount;

			if (extra_count <= 0)
				continue;

			// The sample count of a frame differs by one between SIDs at times: hold the last sample,
			// a gap in the output of one SID is heard as a tick
			for (int i = 0; i < sample_count; ++i)
			{
				const int mixed = static_cast<int>(sample_buffer[i]) + static_cast<int>(extra_samples[i < extra_count ? i : extra_count - 1]);
				sample_buffer[i] = static_cast<short>(mixed > 32767 ? 32767 : (mixed < -32768 ? -32768 : mixed));
			}
		}
	}


	//----------------------------------------------------------------------------------------------------------------
	// Error
	//----------------------------------------------------------------------------------------------------------------

	bool ExecutionHandler::IsInErrorState() const
	{
		return m_ErrorState;
	}

	std::string ExecutionHandler::GetErrorMessage() const
	{
		return m_ErrorMessage;
	}

	void ExecutionHandler::ClearErrorState()
	{
		m_ErrorState = false;
	}


	//----------------------------------------------------------------------------------------------------------------
	// Emulation update
	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::SetEnableUpdate(bool inEnableUpdate)
	{
		Lock();
		m_UpdateEnabled = inEnableUpdate;
		Unlock();
	}

	void ExecutionHandler::SetFastForward(unsigned int inFastForwardUpdateCount)
	{
		Lock();
		m_FastForwardUpdateCount = inFastForwardUpdateCount;
		Unlock();
	}

	void ExecutionHandler::QueueInit(unsigned char inInitArgument)
	{
		Lock();
		m_ActionQueue.push_back({ ActionType::Init, inInitArgument });
		Unlock();
	}

	void ExecutionHandler::QueueInit(unsigned char inInitArgument, const std::function<void(CPUMemory*)>& inPostInitCallback)
	{
		Lock();
		m_ActionQueue.push_back({ ActionType::Init, inInitArgument });
		m_ActionQueue.push_back({ ActionType::Update, 0, inPostInitCallback });
		Unlock();
	}


	void ExecutionHandler::QueueStop()
	{
		Lock();
		m_ActionQueue.push_back({ ActionType::Stop, 0 });
		Unlock();
	}


	void ExecutionHandler::QueueMuteChannel(unsigned char inChannel, const std::function<void(CPUMemory*)>& inMuteCallback)
	{
		Lock();
		m_ActionQueue.push_back({ ActionType::ApplyMuteState, inChannel, inMuteCallback });
		Unlock();
	}


	void ExecutionHandler::QueueClearAllMuteState(const std::function<void(CPUMemory*)>& inClearMuteStateCallback)
	{
		Lock();
		m_ActionQueue.push_back({ ActionType::ClearMuteAllState, 0, inClearMuteStateCallback });
		Unlock();
	}


	void ExecutionHandler::SetInitVector(unsigned short inVector)
	{
		Lock();
		m_InitVector = inVector;
		Unlock();
	}

	void ExecutionHandler::SetStopVector(unsigned short inVector)
	{
		Lock();
		m_StopVector = inVector;
		Unlock();
	}

	void ExecutionHandler::SetUpdateVector(unsigned short inVector)
	{
		Lock();
		m_UpdateVector = inVector;
		Unlock();
	}

	void ExecutionHandler::SetPostUpdateCallback(const std::function<void(CPUMemory*)>& inPostUpdateCallback)
	{
		Lock();
		m_PostUpdateCallback = inPostUpdateCallback;
		Unlock();
	}

	void ExecutionHandler::StartWriteOutputToFile(const std::string& inFilename)
	{
		FOUNDATION_ASSERT(m_SIDProxy != nullptr);
		Lock();
		m_SIDProxy->StartRecordToFile(inFilename);
		Unlock();
	}

	void ExecutionHandler::StopWriteOutputToFile()
	{
		FOUNDATION_ASSERT(m_SIDProxy != nullptr);
		Lock();
		m_SIDProxy->StopRecordToFile();
		Unlock();
	}

	bool ExecutionHandler::IsWritingOutputToFile() const
	{
		FOUNDATION_ASSERT(m_SIDProxy != nullptr);
		return m_SIDProxy->IsRecordingToFile();
	}

	//----------------------------------------------------------------------------------------------------------------

	const unsigned short ExecutionHandler::GetAddressFromActionType(ActionType inActionType) const
	{
		switch (inActionType)
		{
		case ActionType::Init:
			return m_InitVector;
		case ActionType::Stop:
			return m_StopVector;
		case ActionType::Update:
			return m_UpdateVector;
		default:
			break;
		}

		FOUNDATION_ASSERT(false);

		return 0x0000;
	}

	//----------------------------------------------------------------------------------------------------------------

	void ExecutionHandler::SimulateSID(int inDeltaCycles)
	{
		if (m_SkipSIDSimulation)
			return;

		short* pSampleBuffer = static_cast<short*>(m_SampleBuffer);

		//while (inDeltaCycles > 0)
		{
			// Get offset pointer inside the sample buffer
			short* sample_buffer_write_location = pSampleBuffer + m_SampleBufferWriteCursor;

			// Calculate remaining samples in the buffer, so that we do not overflow it!
			const int uiRemainingSamplesInBuffer = m_SampleBufferSize - m_SampleBufferWriteCursor;

			// Clock the sid
			const int nSamplesWritten = m_SIDProxy->Clock(inDeltaCycles, sample_buffer_write_location, uiRemainingSamplesInBuffer);

			// Negative sample count written is invalid!
			FOUNDATION_ASSERT(nSamplesWritten >= 0);

			// Move the write cursor
			m_SampleBufferWriteCursor += static_cast<unsigned int>(nSamplesWritten);

			// Make sure there's no overflow
			FOUNDATION_ASSERT(m_SampleBufferWriteCursor <= m_SampleBufferSize);
		}
	}

	void ExecutionHandler::CaptureNewFrame()
	{
		FOUNDATION_ASSERT(m_CPU != nullptr);

		// Lock execution handler
		Lock();

		// Lock memory access
		m_Memory->Lock();

		// Reset read/write cursor, and run simulation of the SID
		m_SampleBufferReadCursor = 0;
		m_SampleBufferWriteCursor = 0;

		// Attach memory to cpu
		m_CPU->SetMemory(m_Memory);

		// Hardware output replays the cycle distance between writes: stamp writes on their real cycle.
		// Other outputs keep the write stamped at the start of its instruction.
		m_CPU->SetWriteOnLastCycle(m_OutputDevice == ExecutionHandler::OutputDevice::USBSID && m_USBSID != nullptr);

		// Capture the frame (this will run the CPU )
		// Capture the register blocks of all SIDs, SID n sits at $d400 + n * $20
		CPUFrameCapture frameCapture(m_CPU, 0xd400, 0xd400 + MaxSIDCount * 0x20 - 1, m_CyclesPerFrame);

		SyncExtraSIDs();

		// Execute queued actions
		for (const Action& action : m_ActionQueue)
		{
			switch (action.m_ActionType)
			{
			case ActionType::ApplyMuteState:
			{
				// Three voices per SID, seven registers per voice
				const unsigned short offset = (action.m_ActionArgument / 3) * 0x20 + (action.m_ActionArgument % 3) * 7;
				const unsigned short address = 0xd400 + offset;

				for (int i = 0; i < 7; ++i)
					frameCapture.Write(address + i, 0, 0);
			}
			break;
			case ActionType::ClearMuteAllState:
				break;
			case ActionType::Init:
			case ActionType::Stop:
				frameCapture.Capture(GetAddressFromActionType(action.m_ActionType), action.m_ActionArgument);
				break;
			case ActionType::Update:
				if (!m_ErrorState)
					frameCapture.Capture(GetAddressFromActionType(action.m_ActionType), action.m_ActionArgument);
			default:
				break;
			}

			if (action.m_PostActionCallback)
				action.m_PostActionCallback(m_Memory);
		}

		m_ActionQueue.clear();

		// Execute driver update, if enabled
		if (m_UpdateEnabled && !m_ErrorState)
		{
			bool error = frameCapture.IsMaxCycleCountReached();

			if (!error)
			{
				frameCapture.Capture(GetAddressFromActionType(ActionType::Update), 0);
				error = frameCapture.IsMaxCycleCountReached();

				if (m_PostUpdateCallback)
					m_PostUpdateCallback(m_Memory);
			}

			if (!error)
			{
				for (unsigned int i = 0; i < m_FastForwardUpdateCount; ++i)
				{
					// Break out if less than a quarter of the cycles of a frame remains
					if (m_CyclesPerFrame - frameCapture.GetCyclesSpend() < m_CyclesPerFrame >> 2)
						break;

					frameCapture.Capture(GetAddressFromActionType(ActionType::Update), 0);
					if (m_PostUpdateCallback)
						m_PostUpdateCallback(m_Memory);

					error = frameCapture.IsMaxCycleCountReached();

					if (error)
						break;
				}
			}

			if (error)
			{
				m_ErrorState = true;
				m_ErrorMessage = "Emulation of 6510 code exceeded cycle window!";
			}
		}

		// Increment frame counter
		m_CPUFrameCounter++;

		// Run the flight recorder
		if (m_SIDRegisterFlightRecorder != nullptr && m_SIDRegisterFlightRecorder->IsRecording())
		{
			m_SIDRegisterFlightRecorder->Lock();
			m_SIDRegisterFlightRecorder->Record(m_CPUFrameCounter, m_Memory, frameCapture.GetCyclesSpend());
			m_SIDRegisterFlightRecorder->Unlock();
		}

		// Copy sid registers after driver update
		m_Memory->GetData(0xd400, m_SIDRegisterLastDriverUpdate.m_Buffer, sizeof(m_SIDRegisterLastDriverUpdate.m_Buffer));

		// Unlock memory access
		m_Memory->Unlock();

		// Grab the cycle count of the CPU here, as this will be the number of cycles spend on the driver update
		m_CPUCyclesSpend = frameCapture.GetCyclesSpend();

		// Do all writes to the SID and emulate cycles spend
		int nCycle = 0;

		const bool usbsid_output = m_OutputDevice == ExecutionHandler::OutputDevice::USBSID && m_USBSID != nullptr;

		// No samples are needed while the board plays, skip the SID emulation
		m_SkipSIDSimulation = usbsid_output;

		if (usbsid_output)
			m_USBSID->BeginFrame();

		// Collect the writes of the frame, the extra SIDs need theirs before the first SID is clocked
		m_FrameWrites.clear();

		for (auto& extra_sid : m_ExtraSIDs)
			extra_sid->m_Writes.clear();

		while (frameCapture.HasNext())
		{
			const CPUFrameCapture::WriteCapture& capture = frameCapture.GetNext();

			m_FrameWrites.push_back({ capture.m_iCycle, capture.m_usReg, capture.m_ucVal });

			const unsigned char sid_address = static_cast<unsigned char>(capture.m_usReg & 0xff);
			const unsigned int sid_index = sid_address >> 5;
			const unsigned char sid_register = sid_address & 0x1f;

			if (sid_register <= 0x18 && sid_index > 0 && sid_index < m_SIDCount)
				m_ExtraSIDs[sid_index - 1]->m_Writes.push_back({ capture.m_iCycle, sid_register, capture.m_ucVal });
		}

		// Render the extra SIDs next to the first SID. While the board plays, the registers are kept up to date only
		const bool render_extra_sids = !m_SkipSIDSimulation && !m_ExtraSIDs.empty();

		if (render_extra_sids)
		{
			{
				std::lock_guard<std::mutex> lock(m_RenderMutex);
				m_RenderCyclesInFrame = static_cast<int>(m_CyclesPerFrame);
				m_RenderPending = static_cast<unsigned int>(m_ExtraSIDs.size());
				++m_RenderGeneration;
			}

			m_RenderStart.notify_all();
		}
		else
		{
			for (auto& extra_sid : m_ExtraSIDs)
			{
				for (const SIDWrite& write : extra_sid->m_Writes)
					extra_sid->m_SID->Write(static_cast<unsigned char>(write.m_Address), write.m_Value);
			}
		}

		for (const SIDWrite& write : m_FrameWrites)
		{
			FOUNDATION_ASSERT(nCycle <= write.m_Cycle);

			const int deltaCycles = write.m_Cycle - nCycle;
			SimulateSID(deltaCycles);
			nCycle += deltaCycles;

			// Split the address in SID number and register, skip the unused registers behind $18
			const unsigned char sid_address = static_cast<unsigned char>(write.m_Address & 0xff);
			const unsigned int sid_index = sid_address >> 5;
			const unsigned char sid_register = sid_address & 0x1f;

			if (sid_register > 0x18 || sid_index >= m_SIDCount)
				continue;

			if (sid_index == 0)
				m_SIDProxy->Write(sid_register, write.m_Value);

			// ASID carries one SID
			if(sid_index == 0 && m_OutputDevice == ExecutionHandler::OutputDevice::ASID &&  m_ASID != nullptr)
				m_ASID->WriteToSIDRegister(sid_register, write.m_Value);

			// Pass the cycle of the write within the frame, the board replays the exact spacing
			if (usbsid_output)
				m_USBSID->Write(sid_index, sid_register, write.m_Value, write.m_Cycle);
		}

		// Do the rest of the frame
		if(m_ASID != nullptr)
			m_ASID->SendToDevice();

		// Close the frame at its full length, idle cycles after the last write carry into the following frame
		if (usbsid_output)
			m_USBSID->EndFrame(m_CyclesPerFrame);

		while (nCycle < (int)m_CyclesPerFrame)
		{
			const int deltaCycles = m_CyclesPerFrame - nCycle;
			SimulateSID(deltaCycles);
			nCycle += deltaCycles;
		}

		// Add the output of the extra SIDs once their threads are done with the frame
		if (render_extra_sids)
		{
			{
				std::unique_lock<std::mutex> lock(m_RenderMutex);
				m_RenderDone.wait(lock, [&]() { return m_RenderPending == 0; });
			}

			MixExtraSIDs();
		}

		// Reset cycle counter
		m_CurrentCycle = 0;

		// Unlock execution handler
		Unlock();
	}


	void ExecutionHandler::TellSIDWriteOrderInfo(std::vector<Editor::SIDWriteInformation> SIDWriteInfoList)
	{
		if(m_OutputDevice == ExecutionHandler::OutputDevice::ASID && m_ASID != nullptr)
		{
			m_ASID->SendSIDRegisterWriteOrderAndCycleInfo(SIDWriteInfoList);
		}
	}

	void ExecutionHandler::TellSIDEnvironment()
	{
		// The region toggle changes the SID clock only, the board clock follows it
		if (m_USBSID != nullptr)
			m_USBSID->SetPAL(m_SIDProxy->GetEnvironment() == SID_ENVIRONMENT_PAL);

		if(m_OutputDevice == ExecutionHandler::OutputDevice::ASID && m_ASID != nullptr)
		{
			m_ASID->SendSIDEnvironment(m_SIDProxy->GetEnvironment() == SID_ENVIRONMENT_PAL);
			m_ASID->SendSIDType(m_SIDProxy->GetModel() == SID_MODEL_6581);
		}
	}
}
