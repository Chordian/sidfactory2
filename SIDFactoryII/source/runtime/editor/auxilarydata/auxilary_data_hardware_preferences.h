#pragma once

#include "auxilary_data.h"

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

		enum Engine : unsigned char
		{
			RESID,
			ASID
		};

		AuxilaryDataHardwarePreferences();

		void Reset() override;

		const SIDModel GetSIDModel() const;
		void SetSIDModel(const SIDModel inSIDModel);

		const Region GetRegion() const;
		void SetRegion(const Region inRegion);

		const Engine GetEngine() const;
		void SetEngine(const Engine inEngine);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		SIDModel m_SIDModel;
		Region m_Region;
		Engine m_Engine;
	};
}
