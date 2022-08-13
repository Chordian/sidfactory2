#include "copypaste.h"


namespace Editor
{
	CopyPaste& CopyPaste::Instance()
	{
		static CopyPaste data_copy_instance;
		return data_copy_instance;
	}

	CopyPaste::CopyPaste() = default;


	void CopyPaste::Flush()
	{
		m_SequenceEventsData = nullptr;
		m_SequenceData = nullptr;
		m_OrderListData = nullptr;
	}
}
