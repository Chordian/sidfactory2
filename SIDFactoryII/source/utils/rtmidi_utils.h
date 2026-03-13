#pragma once

#include "foundation/base/toptional.h"
#include <string>
#include <vector>

class RtMidiOut;

namespace Utility
{
	namespace RtMidiUtils
	{
		struct RtMidiPortInfo
		{
			unsigned int m_PortNumber;
			std::string m_PortName;
		};

		Foundation::TOptional<RtMidiPortInfo> RtMidi_GetPortInfoByName(const std::vector<RtMidiPortInfo> inMidiPortInfoList, const std::string& inPortName);
		
		std::vector<RtMidiPortInfo> RtMidiOut_GetPorts(RtMidiOut* inRtMidiOut);
		bool RtMidiOut_OpenPort(RtMidiOut* inRtMidiOut, const RtMidiPortInfo& inPort);
		bool RtMidiOut_HasOpenPort(RtMidiOut* inRtMidiOut);
	}
}