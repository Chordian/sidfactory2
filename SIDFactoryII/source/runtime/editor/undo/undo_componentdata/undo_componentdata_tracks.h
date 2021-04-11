#pragma once

#include "runtime/editor/undo/undo_componentdata/undo_componentdata.h"
#include "runtime/editor/undo/undo_datasource/undo_datasource_cpumemory.h"

namespace Editor
{
	class UndoComponentDataTableTracks : public UndoComponentData
	{
	public:
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

		UndoDataSource& GetDataSource() override { return m_Data; }
		const UndoDataSource& GetDataSource() const override { return m_Data; }

	private:
		UndoDataSourceCPUMemory m_Data;
	};
}