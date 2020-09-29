#pragma once

#include "runtime/editor/converters/iconverter.h"

namespace Editor
{
	class ConverterGT : public IConverter
	{
	public:
		ConverterGT();
		virtual ~ConverterGT();

		virtual bool Convert
		(
			void* inData,
			unsigned int inDataSize,
			Foundation::IPlatform* inPlatform,
			ComponentsManager& inComponentsManager,
			std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
		) override;

	private:
		bool CanConvertInput(void* inData, unsigned int inDataSize) const;

		std::shared_ptr<Utility::C64File> m_Result;

		char* m_Data;
		std::function<void(std::shared_ptr<Utility::C64File>)> m_SuccessAction;
	};
}