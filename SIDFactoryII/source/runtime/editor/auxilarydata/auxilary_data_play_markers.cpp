#include "auxilary_data_play_markers.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include "foundation/base/assert.h"

namespace Editor
{
	AuxilaryDataPlayMarkers::AuxilaryDataPlayMarkers()
		: AuxilaryData(Type::PlayMarkers)
		, m_PlayMarkerEventPositionListLayers(std::vector<std::vector<int>>({ std::vector<int>(MaxPlayMarkers, 0) }))
	{

	}


	AuxilaryDataPlayMarkers::~AuxilaryDataPlayMarkers()
	{

	}


	void AuxilaryDataPlayMarkers::Reset()
	{
		m_PlayMarkerEventPositionListLayers = { std::vector<int>(MaxPlayMarkers, 0) };
	}


	const int AuxilaryDataPlayMarkers::GetPlayMarkerEventPosition(int inLayer, int inMarkerIndex) const
	{
		FOUNDATION_ASSERT(inLayer < static_cast<int>(m_PlayMarkerEventPositionListLayers.size()));
		FOUNDATION_ASSERT(inMarkerIndex >= 0 && inMarkerIndex < MaxPlayMarkers);
		return m_PlayMarkerEventPositionListLayers[inLayer][inMarkerIndex];
	}

	void AuxilaryDataPlayMarkers::SetPlayMarkerEventPosition(int inLayer, int inMarkerIndex, int inEventPosition)
	{
		FOUNDATION_ASSERT(inLayer < static_cast<int>(m_PlayMarkerEventPositionListLayers.size()));
		FOUNDATION_ASSERT(inMarkerIndex >= 0 && inMarkerIndex < MaxPlayMarkers);
		m_PlayMarkerEventPositionListLayers[inLayer][inMarkerIndex] = inEventPosition;
	}


	int AuxilaryDataPlayMarkers::GetLayerCount() const
	{
		return static_cast<int>(m_PlayMarkerEventPositionListLayers.size());
	}


	bool AuxilaryDataPlayMarkers::InsertLayer(unsigned int inLayer)
	{
		if (m_PlayMarkerEventPositionListLayers.size() <= inLayer)
		{
			int insert_count = 1 + static_cast<int>(inLayer) - static_cast<int>(m_PlayMarkerEventPositionListLayers.size());

			FOUNDATION_ASSERT(insert_count >= 0);

			for (int i = 0; i < insert_count; ++i)
				m_PlayMarkerEventPositionListLayers.push_back(std::vector<int>(MaxPlayMarkers, 0));
		}
		else
		{
			m_PlayMarkerEventPositionListLayers.insert(m_PlayMarkerEventPositionListLayers.begin() + inLayer, std::vector<int>(MaxPlayMarkers, 0));
		}

		return true;

	}

	bool AuxilaryDataPlayMarkers::RemoveLayer(unsigned int inLayer)
	{
		const int layer_count = static_cast<int>(m_PlayMarkerEventPositionListLayers.size());

		FOUNDATION_ASSERT(m_PlayMarkerEventPositionListLayers.size() > inLayer);
		m_PlayMarkerEventPositionListLayers.erase(m_PlayMarkerEventPositionListLayers.begin() + inLayer);

		return true;
	}

	bool AuxilaryDataPlayMarkers::SwapLayers(unsigned int inLayer1, unsigned int inLayer2)
	{
		const int layer_count = static_cast<int>(m_PlayMarkerEventPositionListLayers.size());

		FOUNDATION_ASSERT(m_PlayMarkerEventPositionListLayers.size() > inLayer1);
		FOUNDATION_ASSERT(m_PlayMarkerEventPositionListLayers.size() > inLayer2);

		const auto layer1_markers = m_PlayMarkerEventPositionListLayers[inLayer1];
		m_PlayMarkerEventPositionListLayers[inLayer1] = m_PlayMarkerEventPositionListLayers[inLayer2];
		m_PlayMarkerEventPositionListLayers[inLayer2] = layer1_markers;

		return true;
	}

	std::vector<unsigned char> AuxilaryDataPlayMarkers::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		const unsigned char layer_count = static_cast<unsigned char>(m_PlayMarkerEventPositionListLayers.size());
		AuxilaryDataUtils::SaveDataPushByte(output, layer_count);
	
		for (unsigned char layer = 0; layer < layer_count; ++layer)
		{
			AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_PlayMarkerEventPositionListLayers[layer].size()));

			for (const int& event_position : m_PlayMarkerEventPositionListLayers[layer])
				AuxilaryDataUtils::SaveDataPushInt(output, event_position);
		}		

		return output;
	}


	unsigned short AuxilaryDataPlayMarkers::GetGeneratedFileVersion() const
	{
		return 2;
	}

	bool AuxilaryDataPlayMarkers::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		m_PlayMarkerEventPositionListLayers.clear();

		if (inDataVersion == 1)
		{
			m_PlayMarkerEventPositionListLayers.push_back(std::vector<int>(MaxPlayMarkers, 0));
			auto& play_marker_even_position_list = m_PlayMarkerEventPositionListLayers.back();

			unsigned char event_marker_count = AuxilaryDataUtils::LoadDataPullByte(it);

			for (unsigned char i = 0; i < event_marker_count; ++i)
			{
				int event_pos = static_cast<int>(AuxilaryDataUtils::LoadDataPullUInt(it));

				if (static_cast<int>(i) < MaxPlayMarkers)
					play_marker_even_position_list[i] = event_pos;
			}
		}
		else if(inDataVersion == 2)
		{
			unsigned char layer_count = AuxilaryDataUtils::LoadDataPullByte(it);

			for (unsigned char layer = 0; layer < layer_count; ++layer)
			{
				m_PlayMarkerEventPositionListLayers.push_back(std::vector<int>(MaxPlayMarkers, 0));
				auto& play_marker_even_position_list = m_PlayMarkerEventPositionListLayers.back();

				unsigned char event_marker_count = AuxilaryDataUtils::LoadDataPullByte(it);

				for (unsigned char i = 0; i < event_marker_count; ++i)
				{
					int event_pos = static_cast<int>(AuxilaryDataUtils::LoadDataPullUInt(it));

					if (static_cast<int>(i) < MaxPlayMarkers)
						play_marker_even_position_list[i] = event_pos;
				}
			}
		}

		return true;
	}
}

