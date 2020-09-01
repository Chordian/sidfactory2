#include "driver_state.h"

namespace Editor
{
	DriverState::DriverState()
		: m_PlayState(PlayState::Stopped)
		, m_PlayNote()
	{

	}


	DriverState::~DriverState()
	{

	}

	bool DriverState::operator == (const DriverState& inOther) const
	{
		if (m_PlayState != inOther.m_PlayState)
			return false;

		return true;
	}

	bool DriverState::operator != (const DriverState& inOther) const
	{
		return !(*this == inOther);
	}


	void DriverState::SetPlayState(PlayState inPlayState)
	{
		m_PlayState = inPlayState;
	}


	const DriverState::PlayState& DriverState::GetPlayState() const
	{
		return m_PlayState;
	}


	void DriverState::SetPlayNote(const PlayNote& inPlayNote)
	{
		m_PlayNote = inPlayNote;
	}


	const DriverState::PlayNote& DriverState::GetPlayNote() const
	{
		return m_PlayNote;
	}
}