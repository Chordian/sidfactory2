#pragma once

namespace Editor
{
	class EditState
	{
	public:
		struct EventHighlight
		{
			int m_FirstEventPos;
			int m_EventPosInterval;

			bool operator == (const EventHighlight& inOther) const
			{
				if (m_FirstEventPos != inOther.m_FirstEventPos)
					return false;
				if (m_EventPosInterval != inOther.m_EventPosInterval)
					return false;

				return true;
			}

			bool operator != (const EventHighlight& inOther) const
			{
				return !(*this == inOther);
			}
		};

		EditState();

		bool operator == (const EditState& inOther) const;
		bool operator != (const EditState& inOther) const;

		const unsigned int GetOctave() const;
		void SetOctave(unsigned int inOctave);

		bool IsSelectedInstrument(const char inInstrument) const;
		void SetSelectedInstrument(const char inInstrument);
		char GetSelectedInstrument() const;

		bool IsSelectedCommand(const char inCommand) const;
		void SetSelectedCommand(const char inCommand);
		char GetSelectedCommand() const;

		bool IsSequenceHighlightingEnabled() const;
		void SetSequenceHighlighting(const bool inEnabled);

		bool IsFollowPlayMode() const;
		void SetFollowPlayMode(bool inPrevent);

		bool IsPreventingSequenceEdit() const;
		void SetPreventSequenceEdit(bool inPreventSequenceEdit);

	private:
		bool m_SequenceHighlightingEnabled;

		unsigned int m_Octave;
		unsigned char m_SelectedInstrument;
		unsigned char m_SelectedCommand;

		bool m_FollowPlayMode;
		bool m_PreventSequenceEdit;

		EventHighlight m_EventHighlight;
	};
}