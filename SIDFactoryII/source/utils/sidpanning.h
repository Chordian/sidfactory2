#pragma once

#include <string>

namespace Utility
{
	// SID v5 panning, flags bits 6-7 (layout) and 8-9 (mode) of the SID file header
	enum class SIDPanLayout : unsigned char
	{
		Standard,
		LCR,
		CenterFirst,
		FullyCentered
	};

	enum class SIDPanMode : unsigned char
	{
		Direct,
		Reverse,
		Group,
		Spread
	};

	// Stereo position of one SID
	enum class SIDPan : unsigned char
	{
		Left,
		Center,
		Right
	};

	static const unsigned int SIDPanMaxSIDCount = 15;

	// Fills outPan[0 .. inSIDCount - 1] with the assignments of the SID file format v5 tables
	void ComputeSIDPanning(SIDPanLayout inLayout, SIDPanMode inMode, unsigned int inSIDCount, SIDPan* outPan);

	// "L-R-L-R" for the given positions
	std::string SIDPanningToString(const SIDPan* inPan, unsigned int inSIDCount);

	const char* GetSIDPanLayoutName(SIDPanLayout inLayout);
	const char* GetSIDPanModeName(SIDPanMode inMode);
	const char* GetSIDPanName(SIDPan inPan);
}
