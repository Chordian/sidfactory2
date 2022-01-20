#include "auxilary_data_songorderlists.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include "foundation/base/assert.h"

namespace Editor
{
	AuxilaryDataSongOrderLists::AuxilaryDataSongOrderLists()
		: AuxilaryData(Type::SongOrderlists)
	{

	}

	AuxilaryDataSongOrderLists::~AuxilaryDataSongOrderLists()
	{

	}

	void AuxilaryDataSongOrderLists::Reset()
	{
		m_SongOrderLists.clear();
	}

	unsigned int AuxilaryDataSongOrderLists::GetSongCount() const
	{
		return static_cast<unsigned int>(m_SongOrderLists.size());
	}


	bool AuxilaryDataSongOrderLists::HasOrderLists(unsigned int inSongIndex) const
	{
		return inSongIndex < GetSongCount();
	}

	const AuxilaryDataSongOrderLists::OrderLists& AuxilaryDataSongOrderLists::GetOrderLists(unsigned int inSongIndex) const
	{
		FOUNDATION_ASSERT(HasOrderLists(inSongIndex));
		return m_SongOrderLists[inSongIndex];
	}

	AuxilaryDataSongOrderLists::OrderLists& AuxilaryDataSongOrderLists::GetOrderLists(unsigned int inSongIndex)
	{
		FOUNDATION_ASSERT(HasOrderLists(inSongIndex));
		return m_SongOrderLists[inSongIndex];
	}


	std::vector<unsigned char> AuxilaryDataSongOrderLists::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		unsigned char song_count = static_cast<unsigned char>(m_SongOrderLists.size());
		AuxilaryDataUtils::SaveDataPushByte(output, song_count);

		for (const auto& song_order_list : m_SongOrderLists)
		{
			unsigned char channel_count = static_cast<unsigned char>(song_order_list.m_OrderLists.size());
			AuxilaryDataUtils::SaveDataPushByte(output, channel_count);

			for (const auto& order_list : song_order_list.m_OrderLists)
				AuxilaryDataUtils::SaveDataPushByteArray(output, order_list);
		}

		return output;
	}


	unsigned short AuxilaryDataSongOrderLists::GetGeneratedFileVersion() const
	{
		return 1;
	}

	bool AuxilaryDataSongOrderLists::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		unsigned char song_count = AuxilaryDataUtils::LoadDataPullByte(it);

		for (unsigned char i = 0; i < song_count; ++i)
		{
			OrderLists order_list;

			unsigned char song_channel_count = AuxilaryDataUtils::LoadDataPullByte(it);

			for (unsigned char j = 0; j < song_channel_count; ++j)
				order_list.m_OrderLists.push_back(AuxilaryDataUtils::LoadDataPullByteArray(it));

			m_SongOrderLists.push_back(order_list);
		}

		return true;
	}
}

