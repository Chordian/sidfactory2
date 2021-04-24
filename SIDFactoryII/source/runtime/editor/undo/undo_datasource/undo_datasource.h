#pragma once

#include "source/runtime/editor/auxilarydata/auxilary_data_table_text.h"

namespace Editor
{
	class UndoDataSource
	{
	public:
		void SetCPUMemoryData(unsigned char* inData)
		{
			m_CPUMemoryData = inData;
		}

		const unsigned char* GetCPUMemoryData() const
		{
			return m_CPUMemoryData;
		}

		void SetAuxilaryDataTableText(const AuxilaryDataTableText& inSource)
		{
			m_AuxilaryDataTableTextData = inSource;
		}

		const AuxilaryDataTableText& GetAuxilaryDataTableText() const
		{
			return m_AuxilaryDataTableTextData;
		}

	private:
		unsigned char* m_CPUMemoryData;
		AuxilaryDataTableText m_AuxilaryDataTableTextData;
	};
}