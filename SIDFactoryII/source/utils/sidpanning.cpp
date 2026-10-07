#include "utils/sidpanning.h"

// Every rule below follows the description of the layout and mode in the SID file format v5
// document (section "SID PANNING CONFIGURATION"), port of USBSID-Player src/file/sidfile.cpp

namespace Utility
{
	namespace
	{
		void Fill(SIDPan* outPan, unsigned int inStart, unsigned int inCount, SIDPan inValue)
		{
			for (unsigned int i = 0; i < inCount; ++i)
				outPan[inStart + i] = inValue;
		}


		void PanDirect(SIDPanLayout inLayout, unsigned int inCount, SIDPan* outPan)
		{
			switch (inLayout)
			{
			case SIDPanLayout::Standard:
				for (unsigned int i = 0; i < inCount; ++i)
					outPan[i] = (i % 2) == 0 ? SIDPan::Left : SIDPan::Right;
				break;
			case SIDPanLayout::LCR:
				{
					static const SIDPan sequence[3] = { SIDPan::Left, SIDPan::Center, SIDPan::Right };

					for (unsigned int i = 0; i < inCount; ++i)
						outPan[i] = sequence[i % 3];
				}
				break;
			case SIDPanLayout::CenterFirst:
				{
					const unsigned int center_count = inCount - (inCount / 3) * 2;
					bool left = true;

					Fill(outPan, 0, center_count, SIDPan::Center);

					for (unsigned int i = center_count; i < inCount; ++i)
					{
						outPan[i] = left ? SIDPan::Left : SIDPan::Right;
						left = !left;
					}
				}
				break;
			case SIDPanLayout::FullyCentered:
				Fill(outPan, 0, inCount, SIDPan::Center);
				break;
			}
		}


		void SwapLeftRight(SIDPan* ioPan, unsigned int inCount)
		{
			for (unsigned int i = 0; i < inCount; ++i)
			{
				if (ioPan[i] == SIDPan::Left)
					ioPan[i] = SIDPan::Right;
				else if (ioPan[i] == SIDPan::Right)
					ioPan[i] = SIDPan::Left;
			}
		}


		// Every left or right position first, in order, then every center position
		void MoveCenterToEnd(SIDPan* ioPan, unsigned int inCount)
		{
			SIDPan sorted[SIDPanMaxSIDCount];
			unsigned int write = 0;

			for (unsigned int i = 0; i < inCount; ++i)
			{
				if (ioPan[i] != SIDPan::Center)
					sorted[write++] = ioPan[i];
			}

			for (unsigned int i = 0; i < inCount; ++i)
			{
				if (ioPan[i] == SIDPan::Center)
					sorted[write++] = ioPan[i];
			}

			for (unsigned int i = 0; i < inCount; ++i)
				ioPan[i] = sorted[i];
		}


		void PanGroup(SIDPanLayout inLayout, unsigned int inCount, SIDPan* outPan)
		{
			if (inLayout == SIDPanLayout::Standard)
			{
				// Two SIDs: L-R. More: groups of two, the last group holds one SID for an odd count
				if (inCount == 2)
				{
					outPan[0] = SIDPan::Left;
					outPan[1] = SIDPan::Right;
					return;
				}

				bool left = true;

				for (unsigned int i = 0; i < inCount; i += 2)
				{
					Fill(outPan, i, inCount - i >= 2 ? 2 : 1, left ? SIDPan::Left : SIDPan::Right);
					left = !left;
				}

				return;
			}

			if (inLayout == SIDPanLayout::LCR)
			{
				// 1-9 SIDs: one group. 10-15 SIDs: a group of 6, then a group of the remaining 4-9
				const unsigned int first_group = inCount <= 9 ? inCount : 6;
				const unsigned int groups[2] = { first_group, inCount - first_group };
				unsigned int position = 0;

				for (unsigned int group : groups)
				{
					const unsigned int side_count = (group + 1) / 3;
					const unsigned int center_count = group - side_count * 2;

					Fill(outPan, position, side_count, SIDPan::Left);
					position += side_count;
					Fill(outPan, position, center_count, SIDPan::Center);
					position += center_count;
					Fill(outPan, position, side_count, SIDPan::Right);
					position += side_count;
				}

				return;
			}

			// Center first
			const unsigned int side_count = inCount / 3;
			const unsigned int center_count = inCount - side_count * 2;
			unsigned int position = center_count;

			Fill(outPan, 0, center_count, SIDPan::Center);

			if (side_count == 1)
			{
				outPan[position++] = SIDPan::Left;
				outPan[position++] = SIDPan::Right;
				return;
			}

			unsigned int remaining_left = side_count;
			unsigned int remaining_right = side_count;

			// Odd count above 1: three of each first, then pairs
			if (side_count % 2 != 0)
			{
				Fill(outPan, position, 3, SIDPan::Left);
				position += 3;
				Fill(outPan, position, 3, SIDPan::Right);
				position += 3;
				remaining_left -= 3;
				remaining_right -= 3;
			}

			bool left = true;

			while (remaining_left > 0 || remaining_right > 0)
			{
				Fill(outPan, position, 2, left ? SIDPan::Left : SIDPan::Right);
				position += 2;

				if (left)
					remaining_left -= 2;
				else
					remaining_right -= 2;

				left = !left;
			}
		}


		void PanSpread(SIDPanLayout inLayout, unsigned int inCount, SIDPan* outPan)
		{
			if (inLayout == SIDPanLayout::Standard)
			{
				// An odd count adds one to the left side
				const unsigned int left_count = (inCount + 1) / 2;

				Fill(outPan, 0, left_count, SIDPan::Left);
				Fill(outPan, left_count, inCount - left_count, SIDPan::Right);
				return;
			}

			const unsigned int side_count = inCount / 3;
			const unsigned int center_count = inCount - side_count * 2;

			if (inLayout == SIDPanLayout::LCR)
			{
				Fill(outPan, 0, side_count, SIDPan::Left);
				Fill(outPan, side_count, center_count, SIDPan::Center);
				Fill(outPan, side_count + center_count, side_count, SIDPan::Right);
			}
			else
			{
				Fill(outPan, 0, center_count, SIDPan::Center);
				Fill(outPan, center_count, side_count, SIDPan::Left);
				Fill(outPan, center_count + side_count, side_count, SIDPan::Right);
			}
		}
	}


	void ComputeSIDPanning(SIDPanLayout inLayout, SIDPanMode inMode, unsigned int inSIDCount, SIDPan* outPan)
	{
		const unsigned int count = inSIDCount > SIDPanMaxSIDCount ? SIDPanMaxSIDCount : inSIDCount;

		if (count == 0)
			return;

		// One SID plays centered in every layout and mode
		if (count == 1 || inLayout == SIDPanLayout::FullyCentered)
		{
			Fill(outPan, 0, count, SIDPan::Center);
			return;
		}

		switch (inMode)
		{
		case SIDPanMode::Direct:
			PanDirect(inLayout, count, outPan);
			break;
		case SIDPanMode::Reverse:
			PanDirect(inLayout, count, outPan);
			SwapLeftRight(outPan, count);

			if (inLayout == SIDPanLayout::CenterFirst)
				MoveCenterToEnd(outPan, count);
			break;
		case SIDPanMode::Group:
			PanGroup(inLayout, count, outPan);
			break;
		case SIDPanMode::Spread:
			PanSpread(inLayout, count, outPan);
			break;
		}
	}


	std::string SIDPanningToString(const SIDPan* inPan, unsigned int inSIDCount)
	{
		std::string text;

		for (unsigned int i = 0; i < inSIDCount; ++i)
		{
			if (i > 0)
				text += "-";

			text += GetSIDPanName(inPan[i]);
		}

		return text;
	}


	const char* GetSIDPanLayoutName(SIDPanLayout inLayout)
	{
		switch (inLayout)
		{
		case SIDPanLayout::Standard:
			return "Standard";
		case SIDPanLayout::LCR:
			return "L/C/R";
		case SIDPanLayout::CenterFirst:
			return "Center first";
		case SIDPanLayout::FullyCentered:
			return "Fully centered";
		}

		return "";
	}


	const char* GetSIDPanModeName(SIDPanMode inMode)
	{
		switch (inMode)
		{
		case SIDPanMode::Direct:
			return "Direct";
		case SIDPanMode::Reverse:
			return "Reverse";
		case SIDPanMode::Group:
			return "Group";
		case SIDPanMode::Spread:
			return "Spread";
		}

		return "";
	}


	const char* GetSIDPanName(SIDPan inPan)
	{
		switch (inPan)
		{
		case SIDPan::Left:
			return "L";
		case SIDPan::Center:
			return "C";
		case SIDPan::Right:
			return "R";
		}

		return "";
	}
}
