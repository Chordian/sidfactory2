#pragma once

#include "runtime/editor/converters/iconverter.h"

namespace Editor
{
	class ConverterJCH : public IConverter
	{
	public:
		ConverterJCH();
		virtual ~ConverterJCH();

		virtual bool Convert
		(
			void* inData,
			unsigned int inDataSize,
			ComponentsManager& inComponentsManager,
			std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
		) override;

	private:
		bool IsFileValid(void* inData, unsigned int inDataSize) const;

		char* m_Data;
		std::function<void(std::shared_ptr<Utility::C64File>)> m_SuccessAction;
	};
}