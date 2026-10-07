#pragma once

#include "auxilary_data.h"
#include "utils/sidpanning.h"

namespace Editor
{
	class AuxilaryDataHardwarePreferences final : public AuxilaryData
	{
	public:
		enum SIDModel : unsigned char
		{
			MOS6581,
			MOS8580
		};

		enum Region : unsigned char
		{
			PAL,
			NTSC
		};

		AuxilaryDataHardwarePreferences();

		void Reset() override;

		const SIDModel GetSIDModel() const;
		void SetSIDModel(const SIDModel inSIDModel);

		const Region GetRegion() const;
		void SetRegion(const Region inRegion);

		// Stereo panning of a multi SID tune, as stored in a SID v5 file header
		const Utility::SIDPanLayout GetPanLayout() const;
		void SetPanLayout(const Utility::SIDPanLayout inPanLayout);

		const Utility::SIDPanMode GetPanMode() const;
		void SetPanMode(const Utility::SIDPanMode inPanMode);

		// Stereo position of a single SID tune, reSID output only
		const Utility::SIDPan GetSingleSIDPan() const;
		void SetSingleSIDPan(const Utility::SIDPan inPan);

		// Stereo position of every SID of a tune with inSIDCount SIDs
		void GetPanning(unsigned int inSIDCount, Utility::SIDPan* outPan) const;

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		void ResetPanning();

		SIDModel m_SIDModel;
		Region m_Region;
		Utility::SIDPanLayout m_PanLayout;
		Utility::SIDPanMode m_PanMode;
		Utility::SIDPan m_SingleSIDPan;
	};
}