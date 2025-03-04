#pragma once

#include "idatasource.h"
#include "runtime/execution/flightrecorder.h"

namespace Editor
{
	class DataSourceFlightRecorder : public IDataSource
	{
	public:
		DataSourceFlightRecorder(Emulation::FlightRecorder* inFlightRecorder);
		virtual ~DataSourceFlightRecorder();

		void Lock();
		void Unlock();

		const Emulation::FlightRecorder::Frame& operator [](unsigned int inIndex) const;
		const Emulation::FlightRecorder::Frame& GetMostRecentFrame() const;
		const int GetSize() const override;
		const bool IsRecording() const;
		const unsigned int GetNewestRecordingIndex() const;
		const unsigned int GetCyclesSpendMax() const;

		bool PushDataToSource() override { return true; }

	protected:
		Emulation::FlightRecorder* m_FlightRecorder;
	};
}
