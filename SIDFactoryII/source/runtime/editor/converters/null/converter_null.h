#pragma once

#include "runtime/editor/converters/converterbase.h"

namespace Editor
{
	class ConverterNull : public ConverterBase
	{
	public:
		ConverterNull();
		virtual ~ConverterNull();

		bool CanConvert(const void* inData, unsigned int inDataSize) const override;
		bool Update() override;

	private:
		void Setup();
	};
}