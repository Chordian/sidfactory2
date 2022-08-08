#include "datacopy_sequence.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataCopySequence::DataCopySequence()
		: m_EventCount(0)
	{
		m_Events = new DataSourceSequence::Event[DataSourceSequence::MaxEventCount];
	}


	DataCopySequence::DataCopySequence(const DataSourceSequence& inDataSource)
		: m_EventCount(inDataSource.GetLength())
	{
		m_Events = new DataSourceSequence::Event[DataSourceSequence::MaxEventCount];

		for (int i = 0; i < static_cast<int>(m_EventCount); ++i)
			m_Events[i] = inDataSource[i];
	}


	DataCopySequence::~DataCopySequence()
	{
		delete[] m_Events;
	}


	unsigned int DataCopySequence::GetEventCount() const
	{
		return m_EventCount;
	}


	const DataSourceSequence::Event& DataCopySequence::operator[](unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex < m_EventCount);

		return m_Events[inIndex];
	}


	DataCopySequenceEvents::DataCopySequenceEvents()
		: m_EventCount(0)
	{
		m_Events = new DataSourceSequence::Event[DataSourceSequence::MaxEventCount];
	}


	DataCopySequenceEvents::DataCopySequenceEvents(const std::vector<DataSourceSequence::Event>& inSequenceEventsList)
		: m_EventCount(static_cast<unsigned int>(inSequenceEventsList.size()))
	{
		m_Events = new DataSourceSequence::Event[DataSourceSequence::MaxEventCount];

		for (int i = 0; i < static_cast<int>(m_EventCount); ++i)
			m_Events[i] = inSequenceEventsList[i];
	}


	DataCopySequenceEvents::~DataCopySequenceEvents()
	{
		delete[] m_Events;
	}


	unsigned int DataCopySequenceEvents::GetEventCount() const
	{
		return m_EventCount;
	}


	const DataSourceSequence::Event& DataCopySequenceEvents::operator[](unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(inIndex < m_EventCount);

		return m_Events[inIndex];
	}
}
