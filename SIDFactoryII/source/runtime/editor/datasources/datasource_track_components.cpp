#include "datasource_track_components.h"
#include <assert.h>

//------------------------------------------------------------------------------------------------

namespace Editor
{
	DataSourceTrackComponents::DataSourceTrackComponents(const std::vector<std::shared_ptr<ComponentTrack>>& inTracks)
		: m_Tracks(inTracks)
	{
	}

	DataSourceTrackComponents::~DataSourceTrackComponents()
	{

	}

	//------------------------------------------------------------------------------------------------

	std::shared_ptr<ComponentTrack>& DataSourceTrackComponents::operator[](int inIndex)
	{
		assert(inIndex >= 0);
		assert(inIndex < static_cast<int>(m_Tracks.size()));

		return m_Tracks[inIndex];
	}

	//------------------------------------------------------------------------------------------------

	const int DataSourceTrackComponents::GetSize() const
	{
		return static_cast<int>(m_Tracks.size());
	}
}