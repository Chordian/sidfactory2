#pragma once

#include "component_base.h"
#include "component_track_utils.h"
#include "runtime/editor/datasources/datasource_track_components.h"
#include "runtime/editor/driver/idriver_architecture.h"

#include <memory>
#include <string>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class ComponentTrack;
	class DataCopySequence;
	class AuxilaryDataCollection;

	class UndoComponentDataTableTracks;

	class ComponentTracks final : public ComponentBase
	{
	public:
		ComponentTracks(
			int inID, 
			int inGroupID, 
			Undo* inUndo, 
			std::shared_ptr<DataSourceTrackComponents> inDataSource, 
			Foundation::TextField* inTextField, 
			const AuxilaryDataCollection& inAuxilaryDataCollection, 
			const EditState& inEditState,
			int inX, 
			int inY, 
			int inHeight);
		~ComponentTracks();

		bool HasDataChange() const override;

		bool MayTabOutOfFocus(bool inForward) const override;

		void SetHasControl(GetControlType inGetControlType, CursorControl& inCursorControl) override;
		void ClearHasControl(CursorControl& inCursorControl) override;

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		bool ConsumeInput(const Foundation::Mouse& inMouse, bool inModifierKeyMask, CursorControl& inCursorControl, ComponentsManager& inComponentsManager) override;
		void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) override;
		
		void Refresh(const DisplayState& inDisplayState) override;
		void HandleDataChange() override;
		void PullDataFromSource(const bool inFromUndo) override;

		void ExecuteInsertDeleteRule(const DriverInfo::TableInsertDeleteRule& inRule, int inSourceTableID, int inIndexPre, int inIndexPost) override;
		void ExecuteAction(int inActionInput) override;

		void ForceRefresh() override;

		const bool IsMuted(int inTrack) const;
		void SetMuted(int inTrack, bool inMuted);

		int GetEventPosition() const;
		void SetEventPosition(int inEventPos, bool inCancelOrderlistFocus);
		int GetEventPositionAtTopOfCurrentSequence() const;
		int GetMaxEventPosition() const;

		int GetFocusTrackIndex() const;
		
		void SetOrderlistLoopPointAll();
		int GetLoopEventPosition() const;

		void CancelFocusModeOrderList();

		void TellPlaybackEventPosition(int inPlaybackEventPosition);
		bool ComputePlaybackStateFromEventPosition(int inEventPos, std::vector<IDriverArchitecture::PlayMarkerInfo>& inPlayMarkerInfoList) const;

		void OnOrderListChanged(int inChannel);

	private:
		void AlignTracks();
		void HandleSequenceSplit(unsigned char inSequence, unsigned char inSequenceToAdd);
		
		void OnTabForward(CursorControl& inCursorControl);
		void OnTabBackward(CursorControl& inCursorControl);

		// Undo
		void AddUndoStep(UndoComponentDataTableTracks& ioData);
		void OnUndo(const UndoComponentDataTableTracks& inData, CursorControl& inCursorControl);

		int m_TracksPositionY;
		int m_TracksHeight;
		
		int m_EventPos;
		int m_MaxEventPos;
		int m_PlaybackEventPosition;

		int m_FocusTrackIndex;
		bool m_FocusModeOrderList;

		const EditState& m_EditState;
		const AuxilaryDataCollection& m_AuxilaryData;
		ComponentTrackUtils::FocusRow m_FocusRow;

		std::shared_ptr<DataSourceTrackComponents> m_DataSource;
	};
}