#pragma once

#include "auxilary_data.h"

namespace Editor
{
	class AuxilaryDataPlayMarkers final : public AuxilaryData
	{
	public:
		static const int MaxPlayMarkers = 8;

		AuxilaryDataPlayMarkers();
		~AuxilaryDataPlayMarkers();

		void Reset() override;

		const int GetPlayMarkerEventPosition(int inMarkerIndex) const;
		void SetPlayMarkerEventPosition(int inMarkerIndex, int inEventPosition);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		std::vector<int> m_PlayMarkerEventPositionList;
	};
}