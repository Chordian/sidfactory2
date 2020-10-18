#pragma once

#include "runtime/editor/converters/converterbase.h"

namespace Editor
{
	class ConverterNull : public ConverterBase
	{
	public:
		ConverterNull();
		virtual ~ConverterNull();

		const std::string GetName() const override;
		bool CanConvert(const void* inData, unsigned int inDataSize) const override;
		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;
		bool Update() override;

	private:
		void Setup() override;
	};
}