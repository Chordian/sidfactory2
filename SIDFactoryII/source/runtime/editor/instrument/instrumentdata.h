#pragma once

#include <memory>
#include <vector>

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class DriverInfo;
	class InstrumentDataTable;
	class ComponentsManager;

	class InstrumentData final
	{
	public:
		static std::shared_ptr<InstrumentData> Create(int inInstrumentIndex, const DriverInfo& inDriverInfo, const ComponentsManager& inComponentManager);
		static std::shared_ptr<InstrumentData> Create(const void* inData, unsigned int inDataSize);
		static bool IsInstrumentData(const void* inData, unsigned int inDataSize);

	private:
		InstrumentData();

	public:
		~InstrumentData();

		std::vector<unsigned char> GetData() const;

	private:
		std::vector<unsigned char> m_InstrumentData;
		std::vector<std::shared_ptr<InstrumentDataTable>> m_InstrumentTableData;
	};
}