#include "auxilary_data_table_text.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include <assert.h>

namespace Editor
{
	AuxilaryDataTableText::AuxilaryDataTableText()
		: AuxilaryData(Type::TableText)
	{

	}


	AuxilaryDataTableText::~AuxilaryDataTableText()
	{

	}


	void AuxilaryDataTableText::Reset()
	{
		m_TableTextLists.clear();
	}

	bool AuxilaryDataTableText::HasText(int inTableID) const
	{
		for (const auto& table_text_list : m_TableTextLists)
		{
			if (table_text_list.m_TableID == inTableID)
				return true;
		}

		return false;
	}


	const std::string AuxilaryDataTableText::GetText(int inTableID, unsigned int inIndex) const
	{
		for (const auto& table_text_list : m_TableTextLists)
		{
			if (table_text_list.m_TableID == inTableID)
			{
				if (inIndex >= 0 && inIndex < table_text_list.m_TextEntries.size())
					return table_text_list.m_TextEntries[inIndex];

				return "";
			}
		}

		return "";
	}


	void AuxilaryDataTableText::SetText(int inTableID, unsigned int inIndex, const std::string& inText)
	{
		TableTextList& table_text_list = [&]() -> TableTextList&
		{
			for (auto& table_text_list : m_TableTextLists)
			{
				if (table_text_list.m_TableID == inTableID)
					return table_text_list;
			}

			m_TableTextLists.push_back(TableTextList({inTableID}));

			return m_TableTextLists.back();
		}();

		if (inIndex < table_text_list.m_TextEntries.size())
			table_text_list.m_TextEntries[inIndex] = inText;
		else
		{
			int empty_insert_count = static_cast<unsigned int>(table_text_list.m_TextEntries.size()) - inIndex;

			for (int i = 0; i < empty_insert_count; ++i)
				table_text_list.m_TextEntries.push_back("");

			table_text_list.m_TextEntries.push_back(inText);
		}
	}


	std::vector<unsigned char> AuxilaryDataTableText::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_TableTextLists.size()));

		for (const auto& table_text_list : m_TableTextLists)
		{
			assert(table_text_list.m_TextEntries.size() < 65536);

			AuxilaryDataUtils::SaveDataPushInt(output, table_text_list.m_TableID);
			AuxilaryDataUtils::SaveDataPushWord(output, static_cast<unsigned short>(table_text_list.m_TextEntries.size()));

			for (const auto& text_entry : table_text_list.m_TextEntries)
				AuxilaryDataUtils::SaveDataPushStdString256(output, text_entry);
		}

		return output;
	}


	unsigned short AuxilaryDataTableText::GetGeneratedFileVersion() const
	{
		return 1;
	}

	bool AuxilaryDataTableText::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		unsigned char entry_count = AuxilaryDataUtils::LoadDataPullByte(it);

		for (int i = 0; i < entry_count; ++i)
		{
			TableTextList table_text_list;

			table_text_list.m_TableID = static_cast<int>(AuxilaryDataUtils::LoadDataPullUInt(it));
			const unsigned short text_entry_count_for_table = AuxilaryDataUtils::LoadDataPullWord(it);

			for (int j = 0; j < text_entry_count_for_table; ++j)
				table_text_list.m_TextEntries.push_back(AuxilaryDataUtils::LoadDataPullStdString256(it));

			m_TableTextLists.push_back(table_text_list);
		}

		return true;
	}
}

