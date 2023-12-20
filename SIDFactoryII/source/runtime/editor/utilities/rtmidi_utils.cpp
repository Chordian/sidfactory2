#include "rtmidi_utils.h"

#include "foundation/base/assert.h"
#include "rtmidi/RtMidi.h"
#include "utils/logging.h"

namespace Editor
{
	namespace RtMidiUtils
	{
		std::optional<const RtMidiPortInfo> RtMidi_GetPortInfoByName(const std::vector<RtMidiPortInfo> inMidiPortInfoList, const std::string& inPortName)
		{
			for (const auto& port_info : inMidiPortInfoList)
			{
				if (port_info.m_PortName == inPortName)
					return port_info;
			}

			return std::nullopt;
		}

		std::vector<RtMidiPortInfo> RtMidiOut_GetPorts(RtMidiOut* inRtMidiOut)
		{
			FOUNDATION_ASSERT(inRtMidiOut != nullptr, "inRtMidiOut must not be nullptr");

			std::vector<RtMidiPortInfo> Output;
			unsigned int available_ports_count = inRtMidiOut->getPortCount();

			for (unsigned int i = 0; i < available_ports_count; ++i)
			{
				try 
				{
					const std::string port_name = inRtMidiOut->getPortName(i);
					Utility::Logging::instance().Info("[FOUND] MIDI output port %d: %s", i, port_name.c_str());

					Output.push_back({ i, port_name });
				}
				catch (RtMidiError &error)
				{
					Utility::Logging::instance().Error("[FOUND] MIDI output port %d: Error: %s", i, error.getMessage().c_str());
				}
			}

			return Output;
		}
		
		bool RtMidiOut_OpenPort(RtMidiOut* inRtMidiOut, const RtMidiPortInfo& inPort)
		{
			FOUNDATION_ASSERT(inRtMidiOut != nullptr, "inRtMidiOut must not be nullptr");

			if(inRtMidiOut->isPortOpen())
			{
				Utility::Logging::instance().Error("[OPEN] Unable to select MIDI output port %d: %s - Midi port is already opened", inPort.m_PortNumber, inPort.m_PortName.c_str());
				return false;
			}
			
			// Choose the port, if available
			unsigned int available_ports_count = inRtMidiOut->getPortCount();
			if(inPort.m_PortNumber >= available_ports_count)
			{
				Utility::Logging::instance().Error("[OPEN] Unable to select MIDI output port %d: %s - The port number exceeds the available number of ports", inPort.m_PortNumber, inPort.m_PortName.c_str());
				return false;
			}

			inRtMidiOut->openPort(inPort.m_PortNumber);
			Utility::Logging::instance().Info("[OPEN] Selecting MIDI output port %d: %s", inPort.m_PortNumber, inPort.m_PortName.c_str());

			return true;
		}

		bool RtMidiOut_HasOpenPort(RtMidiOut* inRtMidiOut)
		{
			FOUNDATION_ASSERT(inRtMidiOut != nullptr, "inRtMidiOut must not be nullptr");
			return inRtMidiOut->isPortOpen();			
		}

	}
}
