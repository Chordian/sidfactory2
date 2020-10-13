#pragma once

#include "runtime/editor/converters/converterbase.h"
#include "runtime/editor/dialog/dialog_base.h"
#include "runtime/editor/datasources/datasource_tlist.h"
#include "runtime/editor/components/component_string_list_selector.h"

namespace SF2
{
	class Interface;
}

namespace Converter
{
	class SourceMod;
}

namespace Editor
{
	class ComponentConsole;
	class ConverterMod : public ConverterBase
	{
	public:
		ConverterMod();
		virtual ~ConverterMod();

		const std::string GetName() const override;
		bool CanConvert(const void* inData, unsigned int inDataSize) const override;
		bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) override;
		bool Update() override;

	private:
		bool CanConvertInput(void* inData, unsigned int inDataSize) const;
		void Setup() override;

		// Console
		std::shared_ptr<ComponentConsole> m_Console;

		// Utility
		std::shared_ptr<SF2::Interface> m_ConversionUtility;

		// Converter
		std::shared_ptr<Converter::SourceMod> m_Converter;

		// Ignore channel index
		unsigned int m_IgnoreChannel;
	};
}