#pragma once

#include "idatasource.h"
#include "runtime/editor/components/component_track.h"

#include <vector>
#include <memory>

namespace Editor
{
class DataSourceTrackComponents final : public IDataSource
	{
	public:
		DataSourceTrackComponents(const std::vector<std::shared_ptr<ComponentTrack>>& inTracks);
		~DataSourceTrackComponents();

		std::shared_ptr<ComponentTrack>& operator[](int inIndex);

		const int GetSize() const override;
		bool PushDataToSource() override { return true; };

	private:
		std::vector<std::shared_ptr<ComponentTrack>> m_Tracks;
	};
}