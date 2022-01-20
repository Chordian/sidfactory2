#include "auxilary_data_editing_preferences.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include "foundation/base/assert.h"

namespace Editor
{
	AuxilaryDataEditingPreferences::AuxilaryDataEditingPreferences()
		: AuxilaryData(Type::EditingPreferences)
	{
		Reset();
	}


	void AuxilaryDataEditingPreferences::Reset()
	{
		m_NotationMode = NotationMode::Sharp;
		m_EventPosHighlightOffset = 0;
		m_EventPosHighlightInterval = 4;
		m_CurrentSong = 0;
	}

	const AuxilaryDataEditingPreferences::NotationMode AuxilaryDataEditingPreferences::GetNotationMode() const
	{
		return m_NotationMode;
	}


	void AuxilaryDataEditingPreferences::SetNotationMode(const NotationMode inNotationMode)
	{
		m_NotationMode = inNotationMode;
	}


	const unsigned char AuxilaryDataEditingPreferences::GetEventPosHighlightOffset() const
	{
		return m_EventPosHighlightOffset;
	}


	void AuxilaryDataEditingPreferences::SetEventPosHighlightOffset(const unsigned char inOffset)
	{
		m_EventPosHighlightOffset = inOffset;
	}


	const unsigned char AuxilaryDataEditingPreferences::GetEventPosHighlightInterval() const
	{
		return m_EventPosHighlightInterval;
	}


	void AuxilaryDataEditingPreferences::SetEventPosHighlightInterval(const unsigned char inInterval)
	{
		m_EventPosHighlightInterval = inInterval;
	}


	const unsigned char AuxilaryDataEditingPreferences::GetCurrentSong() const
	{
		return m_CurrentSong;
	}


	void AuxilaryDataEditingPreferences::SetCurrentSong(const unsigned char inCurrentSong) 
	{
		m_CurrentSong = inCurrentSong;
	}



	std::vector<unsigned char> AuxilaryDataEditingPreferences::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, m_NotationMode);
		AuxilaryDataUtils::SaveDataPushByte(output, m_EventPosHighlightOffset);
		AuxilaryDataUtils::SaveDataPushByte(output, m_EventPosHighlightInterval);
		AuxilaryDataUtils::SaveDataPushByte(output, m_CurrentSong);

		return output;
	}


	unsigned short AuxilaryDataEditingPreferences::GetGeneratedFileVersion() const
	{
		return 2;
	}


	bool AuxilaryDataEditingPreferences::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		if (inDataVersion <= 1)
		{
			m_NotationMode = static_cast<NotationMode>(AuxilaryDataUtils::LoadDataPullByte(it));
			m_EventPosHighlightOffset = AuxilaryDataUtils::LoadDataPullByte(it);
			m_EventPosHighlightInterval = AuxilaryDataUtils::LoadDataPullByte(it);
		}

		if (inDataVersion <= 2)
		{
			m_CurrentSong = AuxilaryDataUtils::LoadDataPullByte(it);
		}

		return true;
	}
}
