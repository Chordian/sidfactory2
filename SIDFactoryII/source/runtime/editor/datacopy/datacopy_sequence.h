#pragma once

#include "runtime/editor/datasources/datasource_sequence.h"

namespace Editor
{
	class DataCopySequence
	{
	public:
		DataCopySequence();
		DataCopySequence(const DataSourceSequence& inDataSource);

		~DataCopySequence();

		unsigned int GetEventCount() const;
		const DataSourceSequence::Event& operator[](unsigned int inIndex) const;

	private:
		unsigned int m_EventCount;
		DataSourceSequence::Event* m_Events;
	};
}