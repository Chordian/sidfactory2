#pragma once

#include "auxilary_data.h"
#include <string>

namespace Editor
{
	class AuxilaryDataSongOrderLists final : public AuxilaryData
	{
	public:
		struct OrderLists
		{
			std::vector<std::vector<unsigned char>> m_OrderLists;
		};

		AuxilaryDataSongOrderLists();
		~AuxilaryDataSongOrderLists();

		void Reset() override;

		unsigned int GetSongCount() const;
		bool HasOrderLists(unsigned int inSongIndex) const;
		const OrderLists& GetOrderLists(unsigned int inSongIndex) const;
		OrderLists& GetOrderLists(unsigned int inSongIndex);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		std::vector<OrderLists> m_SongOrderLists;
	};
}