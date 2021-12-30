#include "auxilary_data_collection.h"

#include "auxilary_data_editing_preferences.h"
#include "auxilary_data_hardware_preferences.h"
#include "auxilary_data_play_markers.h"
#include "auxilary_data_table_text.h"
#include "auxilary_data_songorderlists.h"

#include "utils/c64file.h"

#include "foundation/base/assert.h"

namespace Editor
{
	AuxilaryDataCollection::AuxilaryDataCollection()
		: m_EditingPreferences(std::make_unique<AuxilaryDataEditingPreferences>())
		, m_HardwarePreferences(std::make_unique<AuxilaryDataHardwarePreferences>())
		, m_PlayMarkers(std::make_unique<AuxilaryDataPlayMarkers>())
		, m_TableText(std::make_unique<AuxilaryDataTableText>())
		, m_SongOrderLists(std::make_unique<AuxilaryDataSongOrderLists>())
	{

	}


	AuxilaryDataCollection::~AuxilaryDataCollection()
	{

	}


	void AuxilaryDataCollection::operator=(const AuxilaryDataCollection& inRHS)
	{
		*m_EditingPreferences = *inRHS.m_EditingPreferences;
		*m_HardwarePreferences = *inRHS.m_HardwarePreferences;
		*m_PlayMarkers = *inRHS.m_PlayMarkers;
		*m_TableText = *inRHS.m_TableText;
		*m_SongOrderLists = *inRHS.m_SongOrderLists;
	}


	void AuxilaryDataCollection::Reset()
	{
		m_EditingPreferences->Reset();
		m_HardwarePreferences->Reset();
		m_PlayMarkers->Reset();
		m_TableText->Reset();
		m_SongOrderLists->Reset();
	}


	AuxilaryDataPlayMarkers& AuxilaryDataCollection::GetPlayMarkers()
	{
		return *m_PlayMarkers;
	}


	const AuxilaryDataPlayMarkers& AuxilaryDataCollection::GetPlayMarkers() const
	{
		return *m_PlayMarkers;
	}


	AuxilaryDataHardwarePreferences& AuxilaryDataCollection::GetHardwarePreferences()
	{
		return *m_HardwarePreferences;
	}

	
	const AuxilaryDataHardwarePreferences& AuxilaryDataCollection::GetHardwarePreferences() const
	{
		return *m_HardwarePreferences;
	}


	AuxilaryDataEditingPreferences& AuxilaryDataCollection::GetEditingPreferences()
	{
		return *m_EditingPreferences;
	}


	const AuxilaryDataEditingPreferences& AuxilaryDataCollection::GetEditingPreferences() const
	{
		return *m_EditingPreferences;
	}


	AuxilaryDataTableText& AuxilaryDataCollection::GetTableText()
	{
		return *m_TableText;
	}


	const AuxilaryDataTableText& AuxilaryDataCollection::GetTableText() const
	{
		return *m_TableText;
	}


	AuxilaryDataSongOrderLists& AuxilaryDataCollection::GetSongOrderLists()
	{
		return *m_SongOrderLists;
	}


	const AuxilaryDataSongOrderLists& AuxilaryDataCollection::GetSongOrderLists() const
	{
		return *m_SongOrderLists;
	}

	
	bool AuxilaryDataCollection::Save(Utility::C64FileWriter& inWriter) const
	{
		AuxilaryData end_mark;

		m_PlayMarkers->Write(inWriter);
		m_HardwarePreferences->Write(inWriter);
		m_EditingPreferences->Write(inWriter);
		m_TableText->Write(inWriter);
		m_SongOrderLists->Write(inWriter);

		end_mark.Write(inWriter);

		return true;
	}


	bool AuxilaryDataCollection::Load(Utility::C64FileReader& inReader) const
	{
		AuxilaryData end_mark;

		while (true)
		{
			FOUNDATION_ASSERT(inReader.IsAtReadableAddress());
			AuxilaryData::FileHeader header = AuxilaryData::FileHeader::Read(inReader);

			if (end_mark.Read(header, inReader))
				break;

			bool data_read = false;

			data_read |= m_PlayMarkers->Read(header, inReader);
			data_read |= m_HardwarePreferences->Read(header, inReader);
			data_read |= m_EditingPreferences->Read(header, inReader);
			data_read |= m_TableText->Read(header, inReader);
			data_read |= m_SongOrderLists->Read(header, inReader);

			FOUNDATION_ASSERT(data_read);

			if (!data_read)
				return false;
		}

		return true;
	}
}
