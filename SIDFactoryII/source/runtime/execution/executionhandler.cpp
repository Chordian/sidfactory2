#include "executionhandler.h"

#include "runtime/emulation/cpuframecapture.h"
#include "runtime/emulation/cpumos6510.h"
#include "runtime/emulation/sid/sidproxy.h"

#include "runtime/environmentdefines.h"
#include "runtime/execution/flightrecorder.h"

#include "foundation/base/assert.h"
#include "foundation/platform/imutex.h"
#include "foundation/platform/iplatform.h"
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
		FlightRecorder* inFlightRecorder)
		: m_CPU(inCPU)
		, m_Memory(pMemory)
		, m_SIDProxy(pSIDProxy)
		, m_SIDRegisterFlightRecorder(inFlightRecorder)
		, m_IsStarted(false)
		, m_FeedCount(0)
		, m_BytesFedCount(0)
		, m_SampleBufferReadCursor(0)
		, m_SampleBufferWriteCursor(0)
		, m_CPUFrameCounter(0)
		, m_UpdateEnabled(false)
    , m_ErrorState(false)
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

		// [begin] ------------- ASID - move -------------
		
		// ASID MIDI handling
		std::string config_asid_midi_port_name = GetSingleConfigurationValue<Utility::Config::ConfigValueString>(Global::instance().GetConfig(), "Playback.ASID.MidiInterface", std::string(""));
		m_pRtMidiOut = new RtMidiOut();

		// Check available MIDI outputs
		unsigned int uiAvailablePorts = m_pRtMidiOut->getPortCount();
		unsigned int uiSelectedPort = 0;
		std::string portName;

		for ( unsigned int i = 0; i < uiAvailablePorts; i++ )
		{
			try {
				portName = m_pRtMidiOut->getPortName(i);
				Logging::instance().Info("MIDI output port %d: %s", i, portName.c_str());

				// Check if the port name corresponds to the start of the configured name (as sometimes, the OS adds info after)
				if ((config_asid_midi_port_name.length() > 0) && (portName.rfind(config_asid_midi_port_name, 0) == 0))
				{
					uiSelectedPort = i;
				}
			}
			catch (RtMidiError &error) {
				Logging::instance().Error("MIDI output port %d: Error: %s", i, error.getMessage().c_str());
			}
		}

		// Choose the port, if available
		if ((uiAvailablePorts > 0) && (config_asid_midi_port_name.length() > 0))
		{
			m_pRtMidiOut->openPort(uiSelectedPort);
			std::string selected_midi_port_name = m_pRtMidiOut->getPortName(uiSelectedPort);

			if (selected_midi_port_name.rfind(config_asid_midi_port_name, 0) != 0)
			{
				Logging::instance().Info("Could not find MIDI output '%s'", config_asid_midi_port_name.c_str());
			}
			Logging::instance().Info("Selecting ASID MIDI output port %d: %s", uiSelectedPort, selected_midi_port_name.c_str());
		}
		else if (config_asid_midi_port_name.length() > 0)
		{
			Logging::instance().Info("ASID MIDI output disabled, no MIDI interfaces found");
		}
		else
		{
			Logging::instance().Info("ASID MIDI output disabled by config");
		}

		// Reset the ASID buffer
		for (int i = 0; i < ASID_NUM_REGS; i++)
		{
			m_aucAsidRegisterBuffer[i] = 0;
			m_aucAsidRegisterUpdated[i] = false;
		}

		// [end] ------------- ASID - move -------------
	}

	ExecutionHandler::~ExecutionHandler()
	{
		m_Mutex = nullptr;

		if (m_SampleBuffer != nullptr)
			delete[] m_SampleBuffer;

		// [begin] ------------- ASID - move -------------
		delete m_pRtMidiOut;
		// [end] ------------- ASID - move -------------
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

			if (m_SIDRegisterFlightRecorder != nullptr)
			{
				m_SIDRegisterFlightRecorder->Lock();
				m_SIDRegisterFlightRecorder->Reset();
				m_SIDRegisterFlightRecorder->Unlock();
			}

			m_IsStarted = true;
		}
	}

	void ExecutionHandler::Stop()
	{
		if (m_IsStarted)
		{
			m_SampleBufferReadCursor = 0;
			m_SampleBufferWriteCursor = 0;

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

	void ExecutionHandler::FeedPCM(void* inBuffer, unsigned int inByteCount)
	{
		m_FeedCount++;
		m_BytesFedCount += inByteCount;

		if (!m_IsStarted)
		{
			memset(inBuffer, 0, inByteCount);
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
					const float fSample = static_cast<float>(pSource[i + m_SampleBufferReadCursor]) * m_OutputGain;
					const float fClampedSample = fmin(sampleCeiling, fmax(fSample, sampleFloor));
					pTarget[i] = static_cast<short>(fClampedSample);
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

	// [begin] ------------- ASID - move -------------

	void ExecutionHandler::ASIDWrite(unsigned char ucSidReg, unsigned char ucData)
	{
		if (ucSidReg > 0x18) {
			return;
		}

		// Get the ASID transformed register
		unsigned char ucMappedAddr = GetASIDposFromSIDreg(ucSidReg);

		// If a write occurs to a waveform register, check if first block is already allocated
		if ((ucMappedAddr >= 0x16) && (ucMappedAddr <= 0x18) && m_aucAsidRegisterUpdated[ucMappedAddr])
		{
			ucMappedAddr += 3;

			// If second block is also updated, move it to the first to make sure to always keep the last one
			if( m_aucAsidRegisterUpdated[ucMappedAddr])
			{
				m_aucAsidRegisterBuffer[ucMappedAddr-3] = m_aucAsidRegisterBuffer[ucMappedAddr];
			}
		}

		// If we're trying to update a control register that is already mapped, flush it directly
		if( m_aucAsidRegisterUpdated[ucMappedAddr])
		{
			if( ucMappedAddr >= 0x16)
			{
				ASIDSend();
			}
		}

		// Store the data
		m_aucAsidRegisterBuffer[ucMappedAddr] = ucData;
		m_aucAsidRegisterUpdated[ucMappedAddr] = true;

	}

	void ExecutionHandler::ASIDSend()
	{
		// Physical out buffer, including protocol overhead
		static unsigned char aucAsidOutBuffer[ASID_NUM_REGS+12];

		// Update needed?
		unsigned char ucUpdate = false;
		for (int i = 0; i < ASID_NUM_REGS; i++)
		{
			if (m_aucAsidRegisterUpdated[i])
			{
				ucUpdate = true;
				break;
			}
		}
		if (!ucUpdate)
		{
			return;
		}

		// Sysex start data for an ASID message
		aucAsidOutBuffer[0] = 0xf0;
		aucAsidOutBuffer[1] = 0x2d;
		aucAsidOutBuffer[2] = 0x4e;
		size_t index = 3;

		// Setup mask bytes (one bit per register)
		unsigned char ucReg;
		for (unsigned char ucMask = 0; ucMask<4; ucMask++)
		{
			ucReg = 0x00;
			for (unsigned char ucRegOffset = 0; ucRegOffset < 7; ucRegOffset++)
			{
				if (m_aucAsidRegisterUpdated[ucMask*7+ucRegOffset])
				{
					ucReg |= (1<<ucRegOffset);
				}
			}
			aucAsidOutBuffer[index++] = ucReg;
		}

		// Setup the MSB bits, one per register (since MIDI only allows for 7-bit data bytes)
		for (unsigned char ucMsb=0; ucMsb<4; ucMsb++)
		{
			ucReg = 0x00;
			for (unsigned char ucRegOffset = 0; ucRegOffset < 7; ucRegOffset++)
			{
				if (m_aucAsidRegisterBuffer[ucMsb*7 + ucRegOffset] & 0x80)
				{
					ucReg |= (1 << ucRegOffset);
				}
			}
			aucAsidOutBuffer[index++] = ucReg;
		}

		// Add data for all updated registers (the 7 LSB bits)
		for (int i = 0; i < ASID_NUM_REGS; i++)
		{
			if (m_aucAsidRegisterUpdated[i])
			{
				aucAsidOutBuffer[index++] = m_aucAsidRegisterBuffer[i] & 0x7f;
			}
		}

		// Sysex end marker
		aucAsidOutBuffer[index++] = 0xf7;

		// Send to physical MIDI port
		if (m_pRtMidiOut->isPortOpen())
		{
			m_pRtMidiOut->sendMessage(aucAsidOutBuffer, index);
		}

		// Prepare for next buffer
		for (int i = 0; i < ASID_NUM_REGS; i++)
		{
			m_aucAsidRegisterUpdated[i] = false;
		}
	}

	unsigned char ExecutionHandler::GetASIDposFromSIDreg(unsigned char ucSIDReg)
	{
		// Conversion between SID register and ASID position
		const unsigned char aucAsidRegMap[] = {
			0x00, 0x01, 0x02, 0x03, 0x16, 0x04, 0x05,
			0x06, 0x07, 0x08, 0x09, 0x17, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f, 0x18, 0x10, 0x11,
			0x12, 0x13, 0x14, 0x15, 0x19, 0x1a, 0x1b
		};

		return aucAsidRegMap[ucSIDReg];
	}

	// [end] ------------- ASID - move -------------

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

		// Capture the frame (this will run the CPU )
		CPUFrameCapture frameCapture(m_CPU, 0xd400, 0xd418, m_CyclesPerFrame);

		// Execute queued actions
		for (const Action& action : m_ActionQueue)
		{
			switch (action.m_ActionType)
			{
			case ActionType::ApplyMuteState:
			{
				const unsigned short offset = action.m_ActionArgument * 7;
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

		while (frameCapture.HasNext())
		{
			const CPUFrameCapture::WriteCapture& capture = frameCapture.GetNext();

			FOUNDATION_ASSERT(nCycle <= capture.m_iCycle);

			const int deltaCycles = capture.m_iCycle - nCycle;
			SimulateSID(deltaCycles);
			m_SIDProxy->Write((unsigned char)(capture.m_usReg & 0xff), capture.m_ucVal);
			nCycle += deltaCycles;

			ASIDWrite((unsigned char)(capture.m_usReg & 0xff), capture.m_ucVal);
		}

		// Do the rest of the frame
		ASIDSend();
		while (nCycle < (int)m_CyclesPerFrame)
		{
			const int deltaCycles = m_CyclesPerFrame - nCycle;
			SimulateSID(deltaCycles);
			nCycle += deltaCycles;
		}

		// Reset cycle counter
		m_CurrentCycle = 0;

		// Unlock execution handler
		Unlock();
	}

	void ExecutionHandler::SendASIDWriteOrder(std::vector<Editor::SIDWriteInformation> SIDWriteInfoList)
	{
		#define WRITE_ORDER_UNUSED 0xff
		// Physical out buffer, including protocol overhead
		unsigned char aucAsidOutBuffer[ASID_NUM_REGS*2+4];

		struct write_order
		{
			unsigned char ucIndex;
			unsigned char ucCycles;
		};

		// Write order list, initialized with all slots unused
		std::array<struct write_order, ASID_NUM_REGS> aWriteOrder;
		aWriteOrder.fill({0, WRITE_ORDER_UNUSED});

		// Get the write order from the analysis, for all the three voices
		int index = 0;
		int iPrevASIDreg = -1;
		for (int voice = 0; voice < 3 ; voice++)
		{
			unsigned char ucLastCycleOffset = 0;
			for (const auto& SIDWriteInfo : SIDWriteInfoList)
			{
				unsigned char ucASIDreg;
				ucASIDreg = GetASIDposFromSIDreg((SIDWriteInfo.m_AddressLow & 0xff)+7*voice);
				aWriteOrder[ucASIDreg].ucIndex = index;

				// The cycles are specified as "post wait times", so will be used on the next cycle value
				if (iPrevASIDreg >= 0)
				{
					aWriteOrder[iPrevASIDreg].ucCycles = SIDWriteInfo.m_CycleOffset-ucLastCycleOffset;
					ucLastCycleOffset = SIDWriteInfo.m_CycleOffset;
				}
				iPrevASIDreg = ucASIDreg;

				index++;
			}
		}
		// Last register does not need to wait any cycles
		aWriteOrder[iPrevASIDreg].ucCycles = 0;

		// Add the remaining registers last, without waits
		for (auto& wo : aWriteOrder)
		{
			if (wo.ucCycles == WRITE_ORDER_UNUSED)
			{
				wo.ucIndex = index++;
				wo.ucCycles = 0;
			}
		}

		// Sysex start data for an ASID message
		index = 0;
		aucAsidOutBuffer[index++] = 0xf0;
		aucAsidOutBuffer[index++] = 0x2d;
		aucAsidOutBuffer[index++] = 0x30; // Write order

		// Fill in the write order payload. Only 7 bits may be used in MIDI
		for (auto& wo : aWriteOrder)
		{
			// Index serves double duty as index (5 bits) and msb (1 bit) of cycles
			aucAsidOutBuffer[index++] = (wo.ucIndex & 0x1f) + ((wo.ucCycles & 0x80)>>1);
			aucAsidOutBuffer[index++] = wo.ucCycles & 0x7f;
		}

		// Sysex end marker
		aucAsidOutBuffer[index++] = 0xf7;

		// [begin] ------------- ASID - move -------------

		// Send to physical MIDI port
		if (m_pRtMidiOut->isPortOpen())
		{
			m_pRtMidiOut->sendMessage(aucAsidOutBuffer, index);
		}

		// [end] ------------- ASID - move -------------
	}
}
