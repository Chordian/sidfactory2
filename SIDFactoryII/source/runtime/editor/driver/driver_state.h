#pragma once

namespace Editor
{
	class DriverState
	{
	public:
		enum class PlayState : int
		{
			Playing,
			Playing_Input,
			Stopped
		};

		struct PlayNote
		{
			int m_Note;
			int m_Instrument;
			bool m_ApplyCommand;
			int m_Command;

			PlayNote()
				: m_Note(-1)
				, m_Instrument(0)
				, m_ApplyCommand(false)
				, m_Command(0)
			{

			}
		};

		DriverState();
		~DriverState();

		bool operator == (const DriverState& inOther) const;
		bool operator != (const DriverState& inOther) const;

		void SetPlayState(PlayState inPlayState);
		const PlayState& GetPlayState() const;

		void SetPlayNote(const PlayNote& inPlayNote);
		const PlayNote& GetPlayNote() const;

	private:

		PlayState m_PlayState;
		PlayNote m_PlayNote;
	};
}