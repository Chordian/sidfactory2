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

		bool HasText(int inTableID) const;
		const std::string GetText(int inTableID, unsigned int inIndex) const;
		void SetText(int inTableID, unsigned int inIndex, const std::string& inText);

	protected:
		std::vector<unsigned char> GenerateSaveData() const override;
		unsigned short GetGeneratedFileVersion() const override;

		bool RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData) override;

	private:
		struct TableTextList
		{
			int m_TableID;
			std::vector<std::string> m_TextEntries;
		};

		std::vector<TableTextList> m_TableTextLists;
	};
}