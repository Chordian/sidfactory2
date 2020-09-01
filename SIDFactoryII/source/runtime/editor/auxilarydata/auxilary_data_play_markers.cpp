#include "auxilary_data_play_markers.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include <assert.h>

namespace Editor
{
	AuxilaryDataPlayMarkers::AuxilaryDataPlayMarkers()
		: AuxilaryData(Type::PlayMarkers)
		, m_PlayMarkerEventPositionList(std::vector<int>(MaxPlayMarkers, 0))
	{

	}


	AuxilaryDataPlayMarkers::~AuxilaryDataPlayMarkers()
	{

	}


	void AuxilaryDataPlayMarkers::Reset()
	{
		for (int& marker : m_PlayMarkerEventPositionList)
			marker = 0;
	}


	const int AuxilaryDataPlayMarkers::GetPlayMarkerEventPosition(int inMarkerIndex) const
	{
		assert(inMarkerIndex >= 0 && inMarkerIndex < MaxPlayMarkers);
		return m_PlayMarkerEventPositionList[inMarkerIndex];
	}

	void AuxilaryDataPlayMarkers::SetPlayMarkerEventPosition(int inMarkerIndex, int inEventPosition)
	{
		assert(inMarkerIndex >= 0 && inMarkerIndex < MaxPlayMarkers);
		m_PlayMarkerEventPositionList[inMarkerIndex] = inEventPosition;
	}


	std::vector<unsigned char> AuxilaryDataPlayMarkers::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_PlayMarkerEventPositionList.size()));

		for(const int& event_position : m_PlayMarkerEventPositionList)
			AuxilaryDataUtils::SaveDataPushInt(output, event_position);

		return output;
	}


	unsigned short AuxilaryDataPlayMarkers::GetGeneratedFileVersion() const
	{
		return 1;
	}

	bool AuxilaryDataPlayMarkers::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		unsigned char event_marker_count = AuxilaryDataUtils::LoadDataPullByte(it);
	
		for (size_t i = 0; i < m_PlayMarkerEventPositionList.size(); ++i)
			m_PlayMarkerEventPositionList[i] = 0;
	
		for (unsigned char i = 0; i < event_marker_count; ++i)
		{
			int event_pos = static_cast<int>(AuxilaryDataUtils::LoadDataPullUInt(it));
			
			if (static_cast<int>(i) < MaxPlayMarkers)
				m_PlayMarkerEventPositionList[i] = event_pos;		
		}

		return true;
	}
}

