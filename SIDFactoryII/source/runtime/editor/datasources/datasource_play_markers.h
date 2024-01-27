#pragma once

#include "datasource_tlist.h"
#include <vector>
#include <string>
#include <memory>

namespace Editor
{
	class DisplayState;
	class AuxilaryDataPlayMarkers;
	class AuxilaryDataSongs;

	class DataSourcePlayMarkers : public DataSourceTList<std::string>
	{
	public:
		DataSourcePlayMarkers(const AuxilaryDataPlayMarkers& inAuxilaryData, const AuxilaryDataSongs& inAuxilarySongs, const DisplayState& inDisplayState);

		std::string& operator[](int inIndex) override;

	private:
		const DisplayState& m_DisplayState;
		const AuxilaryDataPlayMarkers& m_AuxilaryDataPlayMarkers;
		const AuxilaryDataSongs& m_AuxilaryDataSongs;
	};
}