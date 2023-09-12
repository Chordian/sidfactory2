#include "edit_state.h"

namespace Editor
{
	EditState::EditState()
		: m_Octave(4)
		, m_SelectedInstrument(0)
		, m_SelectedCommand(0)
		, m_SequenceHighlightingEnabled(false)
		, m_EventHighlight( { 0, 4 })
		, m_FollowPlayMode(false)
		, m_PreventSequenceEdit(false)
	{
	}


	bool EditState::operator == (const EditState& inOther) const
	{
		if (m_Octave != inOther.m_Octave)
			return false;
		if (m_EventHighlight != inOther.m_EventHighlight)
			return false;
		if (m_SequenceHighlightingEnabled != inOther.m_SequenceHighlightingEnabled)
			return false;
		if (m_FollowPlayMode != inOther.m_FollowPlayMode)
			return false;

		return true;
	}


	bool EditState::operator != (const EditState& inOther) const
	{
		return !(*this == inOther);
	}


	const unsigned int EditState::GetOctave() const
	{
		return m_Octave;
	}


	void EditState::SetOctave(unsigned int inOctave)
	{
		m_Octave = inOctave;
	}


	bool EditState::IsSelectedInstrument(const char inInstrument) const
	{
		return m_SequenceHighlightingEnabled && m_SelectedInstrument == inInstrument;
	}


	void EditState::SetSelectedInstrument(const char inInstrument)
	{
		m_SelectedInstrument = inInstrument;
	}


	char EditState::GetSelectedInstrument() const
	{
		return m_SelectedInstrument;
	}


	bool EditState::IsSelectedCommand(const char inCommand) const
	{
		return m_SequenceHighlightingEnabled && m_SelectedCommand == inCommand;
	}


	void EditState::SetSelectedCommand(const char inCommand)
	{
		m_SelectedCommand = inCommand;
	}


	char EditState::GetSelectedCommand() const
	{
		return m_SelectedCommand;
	}


	bool EditState::IsSequenceHighlightingEnabled() const
	{
		return m_SequenceHighlightingEnabled;
	}


	void EditState::SetSequenceHighlighting(const bool inEnabled)
	{
		m_SequenceHighlightingEnabled = inEnabled;
	}


	bool EditState::IsFollowPlayMode() const
	{
		return m_FollowPlayMode;
	}


	void EditState::SetFollowPlayMode(bool inFollowPlayMode)
	{
		m_FollowPlayMode = inFollowPlayMode;
	}

	bool EditState::IsPreventingSequenceEdit() const
	{
		return m_PreventSequenceEdit;
	}

	void EditState::SetPreventSequenceEdit(bool inPreventSequenceEdit)
	{
		m_PreventSequenceEdit = inPreventSequenceEdit;
	}
}