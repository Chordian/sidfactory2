#pragma once

#include "auxilary_data.h"

namespace Editor
{
	class AuxilaryDataSongs final : public AuxilaryData
	{
	public:
		enum NotationMode : unsigned char
		{
			Sharp,
			Flat
		};

		AuxilaryDataSongs();

		void Reset() override;

		const unsigned char GetSongCount() const;
		void SetSongCount(const unsigned char inSongCount);

		const unsigned char GetSelectedSong() const;
		void SetSelectedSong(const unsigned char inSelectedSong);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;
	
	private:
		unsigned char m_SongCount;
		unsigned char m_SelectedSong;
	};
}