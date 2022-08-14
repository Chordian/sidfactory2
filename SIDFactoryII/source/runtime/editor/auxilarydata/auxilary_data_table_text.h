#pragma once

#include "auxilary_data.h"
#include <string>

namespace Editor
{
	class AuxilaryDataTableText final : public AuxilaryData
	{
	public:
		AuxilaryDataTableText();
		~AuxilaryDataTableText();

		void Reset() override;

		bool HasTextBucketForTable(int inTableID) const;

		bool HasText(int inTableID) const;
		const std::string GetText(int inTableID, unsigned int inIndex) const;
		void SetText(int inTableID, unsigned int inIndex, const std::string& inText);

		bool HasText(int inTableID, unsigned int inLayer) const;
		const std::string GetText(int inTableID, unsigned int inIndex, unsigned int inLayer) const;
		void SetText(int inTableID, unsigned int inIndex, const std::string& inText, unsigned int inLayer);

		bool InsertLayer(int inTableID, unsigned int inLayer);
		bool RemoveLayer(int inTableID, unsigned int inLayer);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		struct TableTextList
		{
			int m_TableID;
			std::vector<std::vector<std::string>> m_TextEntryListLayers;
		};

		std::vector<TableTextList> m_TableTextLists;
	};
}