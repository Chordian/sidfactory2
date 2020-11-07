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


	std::vector<unsigned char> AuxilaryDataEditingPreferences::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, m_NotationMode);
		AuxilaryDataUtils::SaveDataPushByte(output, m_EventPosHighlightOffset);
		AuxilaryDataUtils::SaveDataPushByte(output, m_EventPosHighlightInterval);

		return output;
	}


	unsigned short AuxilaryDataEditingPreferences::GetGeneratedFileVersion() const
	{
		return 1;
	}


	bool AuxilaryDataEditingPreferences::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		m_NotationMode = static_cast<NotationMode>(AuxilaryDataUtils::LoadDataPullByte(it));
		m_EventPosHighlightOffset = AuxilaryDataUtils::LoadDataPullByte(it);
		m_EventPosHighlightInterval = AuxilaryDataUtils::LoadDataPullByte(it);

		return true;
	}
}
