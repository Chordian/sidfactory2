#pragma once

#include "runtime/editor/undo/undo_componentdata.h"

namespace Editor
{
	struct UndoComponentDataTableTracks : public UndoComponentData
	{
		// Tracks
		int m_TracksEventPos;
		int m_TracksMaxEventPos;

		bool m_TracksFocusModeOrderList;
		int m_FocusTrackIndex;

		// Track
		int m_CursorPos;
		int m_MaxEventPos;
		int m_EventPosDetailsOrderListIndex;
		int m_EventPosDetailsSequenceIndex;
		int m_EventPosOrderListIndex;
		int m_EventPosSequenceIndex;

		// Order list input focus
		bool m_FocusModeOrderList;
	};
}