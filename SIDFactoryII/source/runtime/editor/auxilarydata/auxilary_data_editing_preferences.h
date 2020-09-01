#pragma once

#include "auxilary_data.h"

namespace Editor
{
	class AuxilaryDataEditingPreferences final : public AuxilaryData
	{
	public:
		enum NotationMode : unsigned char
		{
			Sharp,
			Flat
		};

		AuxilaryDataEditingPreferences();

		void Reset() override;

		const NotationMode GetNotationMode() const;
		void SetNotationMode(const NotationMode inNotationMode);

		const unsigned char GetEventPosHighlightOffset() const;
		void SetEventPosHighlightOffset(const unsigned char inOffset);

		const unsigned char GetEventPosHighlightInterval() const;
		void SetEventPosHighlightInterval(const unsigned char inInterval);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;
	
	private:
		NotationMode m_NotationMode;
		unsigned char m_EventPosHighlightOffset;
		unsigned char m_EventPosHighlightInterval;
	};
}