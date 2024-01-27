#pragma once

#include "idatasource.h"
#include "runtime/execution/executionhandler.h"
#include "runtime/execution/flightrecorder.h"

namespace Editor
{
	class DataSourceSIDRegistersBufferAfLastDriverUpdate : public IDataSource
	{
	public:
		DataSourceSIDRegistersBufferAfLastDriverUpdate(Emulation::ExecutionHandler* inExecutionHandler);
		virtual ~DataSourceSIDRegistersBufferAfLastDriverUpdate();
		
		const unsigned char operator [](unsigned int inIndex) const;
		const int GetSize() const override;

		bool PushDataToSource() override { return true; }
		void PullDataFromSource();

	protected:
		Emulation::ExecutionHandler* m_ExecutionHandler;
		Emulation::ExecutionHandler::SIDRegistersBuffer m_SIDRegistersBuffer;
	};
}
