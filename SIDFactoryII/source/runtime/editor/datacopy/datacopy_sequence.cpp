#include "datacopy_sequence.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataCopySequence::DataCopySequence()
		: m_EventCount(0)
		, m_IsFullSequenceCopy(false)
	{
		m_Events = new DataSourceSequence::Event[DataSourceSequence::MaxEventCount];
	}


	DataCopySequence::DataCopySequence(const DataSourceSequence& inDataSource)
		: m_EventCount(inDataSource.GetLength())
		, m_IsFullSequenceCopy(true)
	{
		m_Events = new DataSourceSequence::Event[DataSourceSequence::MaxEventCount];

		for (int i = 0; i < static_cast<int>(m_EventCount); ++i)
			m_Events[i] = inDataSource[i];
	}


	DataCopySequence::~DataCopySequence()
	{
		delete[] m_Events;
	}


	bool DataCopySequence::IsFullSequenceCopy() const
	{
		return m_IsFullSequenceCopy;
	}


	unsigned int DataCopySequence::GetEventCount() const
	{
		return m_EventCount;
	}


	const DataSourceSequence::Event& DataCopySequence::operator[](unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex >= 0);
		FOUNDATION_ASSERT(inIndex < m_EventCount);

		return m_Events[inIndex];
	}
}
