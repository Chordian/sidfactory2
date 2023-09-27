#include "datasource_sidregistersbuffer.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceSIDRegistersBufferAfLastDriverUpdate::DataSourceSIDRegistersBufferAfLastDriverUpdate(Emulation::ExecutionHandler* inExecutionHandler)
		: m_ExecutionHandler(inExecutionHandler)
	{

	}


	DataSourceSIDRegistersBufferAfLastDriverUpdate::~DataSourceSIDRegistersBufferAfLastDriverUpdate()
	{

	}


	const unsigned char DataSourceSIDRegistersBufferAfLastDriverUpdate::operator [](unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex < sizeof(m_SIDRegistersBuffer.m_Buffer));

		return m_SIDRegistersBuffer.m_Buffer[inIndex];
	}

	const int DataSourceSIDRegistersBufferAfLastDriverUpdate::GetSize() const
	{
		return static_cast<int>(sizeof(m_SIDRegistersBuffer.m_Buffer));
	}

	void DataSourceSIDRegistersBufferAfLastDriverUpdate::PullDataFromSource()
	{
		FOUNDATION_ASSERT(m_ExecutionHandler != nullptr);
		
		m_ExecutionHandler->Lock();
		m_SIDRegistersBuffer = m_ExecutionHandler->GetSIDRegistersBufferAfterLastDriverUpdate();
		m_ExecutionHandler->Unlock();
	}
}

