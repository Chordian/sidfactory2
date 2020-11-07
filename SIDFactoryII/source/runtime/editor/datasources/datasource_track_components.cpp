#include "datasource_track_components.h"
#include "foundation/base/assert.h"

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
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < static_cast<int>(m_Tracks.size()));

		return m_Tracks[inIndex];
	}

	//------------------------------------------------------------------------------------------------

	const int DataSourceTrackComponents::GetSize() const
	{
		return static_cast<int>(m_Tracks.size());
	}
}