#pragma once

#include "runtime/editor/converters/iconverter.h"

namespace Editor
{
	class ConverterNull : public IConverter
	{
	public:
		ConverterNull();
		virtual ~ConverterNull();

		virtual bool Convert
		(
			void* inData,
			unsigned int inDataSize,
			Foundation::IPlatform* inPlatform,
			ComponentsManager& inComponentsManager,
			std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
		) override;

	private:
		char* m_Data;
		std::function<void(std::shared_ptr<Utility::C64File>)> m_SuccessAction;
	};
}