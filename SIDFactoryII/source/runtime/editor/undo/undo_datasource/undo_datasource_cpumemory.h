#pragma once

#include "source/runtime/editor/undo/undo_datasource/undo_datasource.h"

namespace Editor
{
	class UndoDataSourceCPUMemory : public UndoDataSource
	{
	public:
		UndoDataSourceCPUMemory() : m_Data(nullptr)
		{

		}

		~UndoDataSourceCPUMemory()
		{
			delete[] m_Data;
		}

		void SetData(unsigned char* inData)
		{
			m_Data = inData;
		}

		const unsigned char* GetData() const
		{
			return m_Data;
		}

		Type GetType() const override { return Type::CPUMemory; }

	private:
		unsigned char* m_Data;
	};
}
