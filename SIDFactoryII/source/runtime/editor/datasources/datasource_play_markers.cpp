#include "datasource_play_markers.h"
#include "runtime/editor/auxilarydata/auxilary_data_play_markers.h"
#include "runtime/editor/utilities/editor_utils.h"
#include "runtime/editor/display_state.h"

namespace Editor
{
	DataSourcePlayMarkers::DataSourcePlayMarkers(const AuxilaryDataPlayMarkers& inAuxilaryData, const DisplayState& inDisplayState)
		: DataSourceTList<std::string>(std::vector<std::string>(AuxilaryDataPlayMarkers::MaxPlayMarkers))
		, m_DisplayState(inDisplayState)
		, m_AuxilaryDataPlayMarkers(inAuxilaryData)
	{

	}

	std::string& DataSourcePlayMarkers::operator[](int inIndex)
	{
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < AuxilaryDataPlayMarkers::MaxPlayMarkers);

		const int event_pos = m_AuxilaryDataPlayMarkers.GetPlayMarkerEventPosition(inIndex);
		const bool is_uppercase = m_DisplayState.IsHexUppercase();

		m_List[inIndex] = EditorUtils::ConvertToHexValue(static_cast<unsigned char>(inIndex + 1), is_uppercase) + ": " + EditorUtils::ConvertToHexValue(static_cast<unsigned short>(event_pos), is_uppercase);
		return m_List[inIndex];
	}
}