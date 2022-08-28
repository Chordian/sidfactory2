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

		m_SongNames.clear();
		m_SongNames.push_back("");
	}

	const unsigned char AuxilaryDataSongs::GetSongCount() const
	{
		return m_SongCount;
	}

	void AuxilaryDataSongs::AddSong(const unsigned char inIndex)
	{
		FOUNDATION_ASSERT(inIndex <= m_SongNames.size());

		if (inIndex == m_SongNames.size())
			m_SongNames.push_back("");
		else
			m_SongNames.insert(m_SongNames.begin() + inIndex, "");

		m_SongCount = static_cast<unsigned char>(m_SongNames.size());
	}

	void AuxilaryDataSongs::RemoveSong(const unsigned char inIndex)
	{
		FOUNDATION_ASSERT(inIndex < m_SongNames.size());
		m_SongNames.erase(m_SongNames.begin() + inIndex);

		m_SongCount = static_cast<unsigned char>(m_SongNames.size());
	}

	void AuxilaryDataSongs::SwapSongs(const unsigned char inIndex1, const unsigned char inIndex2)
	{
		FOUNDATION_ASSERT(inIndex1 < m_SongNames.size());
		FOUNDATION_ASSERT(inIndex2 < m_SongNames.size());
		
		if (inIndex1 != inIndex2)
		{
			const auto index1_song_name = m_SongNames[inIndex1];
			m_SongNames[inIndex1] = m_SongNames[inIndex2];
			m_SongNames[inIndex2] = index1_song_name;
		}
	}

	const std::string& AuxilaryDataSongs::GetSongName(const unsigned char inIndex) const
	{
		FOUNDATION_ASSERT(inIndex < m_SongNames.size());
		return m_SongNames[inIndex];
	}

	void AuxilaryDataSongs::SetSongName(const unsigned char inIndex, const std::string& inName)
	{
		FOUNDATION_ASSERT(inIndex < m_SongNames.size());
		m_SongNames[inIndex] = inName;
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

		for (unsigned char i = 0; i < m_SongCount; ++i)
			AuxilaryDataUtils::SaveDataPushStdString256(output, m_SongNames[i]);

		return output;
	}


	unsigned short AuxilaryDataSongs::GetGeneratedFileVersion() const
	{
		return 2;
	}


	bool AuxilaryDataSongs::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		m_SongNames.clear();

		m_SongCount = AuxilaryDataUtils::LoadDataPullByte(it);
		m_SelectedSong = AuxilaryDataUtils::LoadDataPullByte(it);

		if (inDataVersion == 1)
		{
			for (unsigned char i = 0; i < m_SongCount; ++i)
				m_SongNames.push_back("");
		}
		else 	
		{
			for (unsigned char i = 0; i < m_SongCount; ++i)
				m_SongNames.push_back(AuxilaryDataUtils::LoadDataPullStdString256(it));
		}

		return true;
	}
}
