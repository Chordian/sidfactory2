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

		const int GetPlayMarkerEventPosition(int inLayer, int inMarkerIndex) const;
		void SetPlayMarkerEventPosition(int inLayer, int inMarkerIndex, int inEventPosition);

		int GetLayerCount() const;

		bool InsertLayer(unsigned int inLayer);
		bool RemoveLayer(unsigned int inLayer);
		bool SwapLayers(unsigned int inLayer1, unsigned int inLayer2);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		std::vector<std::vector<int>> m_PlayMarkerEventPositionListLayers;
	};
}