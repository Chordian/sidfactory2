#pragma once

#include "auxilary_data.h"
#include <string>

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

		void AddSong(const unsigned char inIndex);
		void RemoveSong(const unsigned char inIndex);
		void SwapSongs(const unsigned char inIndex1, const unsigned char inIndex2);

		const std::string& GetSongName(const unsigned char inIndex) const;
		void SetSongName(const unsigned char inIndex, const std::string& inName);

		const unsigned char GetSelectedSong() const;
		void SetSelectedSong(const unsigned char inSelectedSong);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;
	
	private:
		unsigned char m_SongCount;
		unsigned char m_SelectedSong;
		std::vector<std::string> m_SongNames;
	};
}