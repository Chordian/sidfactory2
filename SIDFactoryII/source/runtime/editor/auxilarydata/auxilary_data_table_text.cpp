#include "auxilary_data_table_text.h"
#include "auxilary_data_utils.h"
#include "utils/c64file.h"
#include "foundation/base/assert.h"

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


	bool AuxilaryDataTableText::HasTextBucketForTable(int inTableID) const
	{
		for (const auto& table_text_list : m_TableTextLists)
		{
			if (table_text_list.m_TableID == inTableID)
				return true;
		}

		return false;
	}


	bool AuxilaryDataTableText::HasText(int inTableID) const
	{
		return HasText(inTableID, 0);
	}


	const std::string AuxilaryDataTableText::GetText(int inTableID, unsigned int inIndex) const
	{
		return GetText(inTableID, inIndex, 0);
	}


	void AuxilaryDataTableText::SetText(int inTableID, unsigned int inIndex, const std::string& inText)
	{
		SetText(inTableID, inIndex, inText, 0);
	}


	bool AuxilaryDataTableText::HasText(int inTableID, unsigned int inLayer) const
	{
		for (const auto& table_text_list : m_TableTextLists)
		{
			if (table_text_list.m_TableID == inTableID)
				return table_text_list.m_TextEntryListLayers.size() > inLayer;
		}

		return false;
	}


	const std::string AuxilaryDataTableText::GetText(int inTableID, unsigned int inIndex, unsigned int inLayer) const
	{
		for (const auto& table_text_list : m_TableTextLists)
		{
			if (table_text_list.m_TableID == inTableID)
			{
				// TODO: Look into layer if it exists!
				if (table_text_list.m_TextEntryListLayers.size() > inLayer)
				{
					if (inIndex >= 0 && inIndex < table_text_list.m_TextEntryListLayers[inLayer].size())
						return table_text_list.m_TextEntryListLayers[inLayer][inIndex];
				}

				return "";
			}
		}

		return "";
	}


	void AuxilaryDataTableText::SetText(int inTableID, unsigned int inIndex, const std::string& inText, unsigned int inLayer)
	{
		auto table_text_list_layer = [&](unsigned int inLayer) -> std::vector<std::string>&
		{
			TableTextList& table_text_list = [&]() -> TableTextList&
			{
				// TODO: Upgrade this lambda to handle layers
				for (auto& table_text_list : m_TableTextLists)
				{
					if (table_text_list.m_TableID == inTableID)
						return table_text_list;
				}

				m_TableTextLists.push_back(TableTextList({ inTableID }));
				return m_TableTextLists.back();
			}();

			if (table_text_list.m_TextEntryListLayers.size() > inLayer)
				return table_text_list.m_TextEntryListLayers[inLayer];

			int insert_count = 1 + static_cast<int>(inLayer) - static_cast<int>(table_text_list.m_TextEntryListLayers.size());

			FOUNDATION_ASSERT(insert_count >= 0);

			for (int i = 0; i < insert_count; ++i)
				table_text_list.m_TextEntryListLayers.push_back(std::vector<std::string>());

			return table_text_list.m_TextEntryListLayers.back();
		};

		auto& table_text_list = table_text_list_layer(inLayer);

		if (inIndex < table_text_list.size())
			table_text_list[inIndex] = inText;
		else
		{
			int empty_insert_count = inIndex - static_cast<int>(table_text_list.size());

			FOUNDATION_ASSERT(empty_insert_count >= 0);

			for (int i = 0; i < empty_insert_count; ++i)
				table_text_list.push_back("");

			table_text_list.push_back(inText);
		}
	}


	bool AuxilaryDataTableText::InsertLayer(int inTableID, unsigned int inLayer)
	{
		TableTextList& table_text_list = [&]() -> TableTextList&
		{
			// TODO: Upgrade this lambda to handle layers
			for (auto& table_text_list : m_TableTextLists)
			{
				if (table_text_list.m_TableID == inTableID)
					return table_text_list;
			}

			m_TableTextLists.push_back(TableTextList({ inTableID }));
			return m_TableTextLists.back();
		}();

		const int layer_count = static_cast<int>(table_text_list.m_TextEntryListLayers.size());

		if (table_text_list.m_TextEntryListLayers.size() <= inLayer)
		{
			int insert_count = 1 + static_cast<int>(inLayer) - static_cast<int>(table_text_list.m_TextEntryListLayers.size());

			FOUNDATION_ASSERT(insert_count >= 0);

			for (int i = 0; i < insert_count; ++i)
				table_text_list.m_TextEntryListLayers.push_back(std::vector<std::string>());
		}
		else
		{	
			table_text_list.m_TextEntryListLayers.insert(table_text_list.m_TextEntryListLayers.begin() + inLayer, std::vector<std::string>());
		}

		return true;
	}

	bool AuxilaryDataTableText::RemoveLayer(int inTableID, unsigned int inLayer)
	{
		for (auto& table_text_list : m_TableTextLists)
		{
			if (table_text_list.m_TableID == inTableID)
			{
				const int layer_count = static_cast<int>(table_text_list.m_TextEntryListLayers.size());

				FOUNDATION_ASSERT(table_text_list.m_TextEntryListLayers.size() > inLayer);
				table_text_list.m_TextEntryListLayers.erase(table_text_list.m_TextEntryListLayers.begin() + inLayer);

				return true;
			}
		}

		return false;
	}


	std::vector<unsigned char> AuxilaryDataTableText::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_TableTextLists.size()));

		for (const auto& table_text_list : m_TableTextLists)
		{
			FOUNDATION_ASSERT(table_text_list.m_TextEntryListLayers.size() < 65536);

			AuxilaryDataUtils::SaveDataPushInt(output, table_text_list.m_TableID);
			AuxilaryDataUtils::SaveDataPushWord(output, static_cast<unsigned short>(table_text_list.m_TextEntryListLayers.size()));

			for (const auto& text_entry_layer : table_text_list.m_TextEntryListLayers)
			{
				AuxilaryDataUtils::SaveDataPushWord(output, static_cast<unsigned short>(text_entry_layer.size()));
				for(const auto& text_entry : text_entry_layer)
					AuxilaryDataUtils::SaveDataPushStdString256(output, text_entry);
			}
		}

		return output;
	}


	unsigned short AuxilaryDataTableText::GetGeneratedFileVersion() const
	{
		return 2;
	}

	bool AuxilaryDataTableText::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		if (inDataVersion == 1)
		{
			unsigned char entry_count = AuxilaryDataUtils::LoadDataPullByte(it);

			for (int i = 0; i < entry_count; ++i)
			{
				TableTextList table_text_list;

				table_text_list.m_TableID = static_cast<int>(AuxilaryDataUtils::LoadDataPullUInt(it));
				const unsigned short text_entry_count_for_table = AuxilaryDataUtils::LoadDataPullWord(it);

				table_text_list.m_TextEntryListLayers.push_back(std::vector<std::string>());
				auto& table_layer_text_list = table_text_list.m_TextEntryListLayers.back();

				for (int j = 0; j < text_entry_count_for_table; ++j)
					table_layer_text_list.push_back(AuxilaryDataUtils::LoadDataPullStdString256(it));

				m_TableTextLists.push_back(table_text_list);
			}
		}
		else if(inDataVersion == 2)
		{
			unsigned char entry_count = AuxilaryDataUtils::LoadDataPullByte(it);

			for (int i = 0; i < entry_count; ++i)
			{
				TableTextList table_text_list;

				table_text_list.m_TableID = static_cast<int>(AuxilaryDataUtils::LoadDataPullUInt(it));
				const unsigned short text_layer_count_for_table = AuxilaryDataUtils::LoadDataPullWord(it);

				for (unsigned short j = 0; j < text_layer_count_for_table; ++j)
				{
					table_text_list.m_TextEntryListLayers.push_back(std::vector<std::string>());
					auto& table_layer_text_list = table_text_list.m_TextEntryListLayers.back();

					const unsigned short text_entry_count_for_table = AuxilaryDataUtils::LoadDataPullWord(it);

					for (unsigned short k = 0; k < text_entry_count_for_table; ++k)
						table_layer_text_list.push_back(AuxilaryDataUtils::LoadDataPullStdString256(it));
				}

				m_TableTextLists.push_back(table_text_list);
			}
		}
		else
		{
			FOUNDATION_ASSERT(false);
			return false;
		}

		return true;
	}
}

