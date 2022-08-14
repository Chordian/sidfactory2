#include "auxilary_data_songs.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include "foundation/base/assert.h"

namespace Editor
{
	AuxilaryDataSongs::AuxilaryDataSongs()
		: AuxilaryData(Type::Songs)

	{
		Reset();
	}


	void AuxilaryDataSongs::Reset()
	{
		m_SongCount = 1;
		m_SelectedSong = 0;
	}

	const unsigned char AuxilaryDataSongs::GetSongCount() const
	{
		return m_SongCount;
	}


	void AuxilaryDataSongs::SetSongCount(const unsigned char inSongCount)
	{
		FOUNDATION_ASSERT(inSongCount > 0);
		m_SongCount = inSongCount;
	}


	const unsigned char AuxilaryDataSongs::GetSelectedSong() const
	{
		return m_SelectedSong;
	}


	void AuxilaryDataSongs::SetSelectedSong(const unsigned char inSelectedSong)
	{
		m_SelectedSong = inSelectedSong;
	}


	std::vector<unsigned char> AuxilaryDataSongs::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, m_SongCount);
		AuxilaryDataUtils::SaveDataPushByte(output, m_SelectedSong);

		return output;
	}


	unsigned short AuxilaryDataSongs::GetGeneratedFileVersion() const
	{
		return 1;
	}


	bool AuxilaryDataSongs::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		m_SongCount = AuxilaryDataUtils::LoadDataPullByte(it);
		m_SelectedSong = AuxilaryDataUtils::LoadDataPullByte(it);

		return true;
	}
}
