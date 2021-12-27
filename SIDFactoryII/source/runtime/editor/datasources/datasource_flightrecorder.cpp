#include "datasource_flightrecorder.h"
#include "foundation/base/assert.h"

namespace Editor
{
	DataSourceFlightRecorder::DataSourceFlightRecorder(Emulation::FlightRecorder* inFlightRecorder)
		: m_FlightRecorder(inFlightRecorder)
	{

	}


	DataSourceFlightRecorder::~DataSourceFlightRecorder()
	{

	}


	void DataSourceFlightRecorder::Lock()
	{
		FOUNDATION_ASSERT(m_FlightRecorder != nullptr);
		m_FlightRecorder->Lock();
	}


	void DataSourceFlightRecorder::Unlock()
	{
		FOUNDATION_ASSERT(m_FlightRecorder != nullptr);
		m_FlightRecorder->Unlock();
	}

	
	const Emulation::FlightRecorder::Frame& DataSourceFlightRecorder::operator [](unsigned int inIndex) const
	{
		FOUNDATION_ASSERT(m_FlightRecorder != nullptr);
		FOUNDATION_ASSERT(inIndex < m_FlightRecorder->GetCapacity());

		return m_FlightRecorder->GetFrame(inIndex);
	}


	const bool DataSourceFlightRecorder::IsRecording() const
	{
		FOUNDATION_ASSERT(m_FlightRecorder != nullptr);
		return m_FlightRecorder->IsRecording();
	}



	const int DataSourceFlightRecorder::GetSize() const
	{
		FOUNDATION_ASSERT(m_FlightRecorder != nullptr);
		return static_cast<int>(m_FlightRecorder->GetCapacity());
	}


	const unsigned int DataSourceFlightRecorder::GetNewestRecordingIndex() const
	{
		FOUNDATION_ASSERT(m_FlightRecorder != nullptr);

		unsigned int index = m_FlightRecorder->RecordedFrameCount();
		if (index >= m_FlightRecorder->GetCapacity())
			return m_FlightRecorder->GetCapacity() - 1;

		return index;
	}
}

