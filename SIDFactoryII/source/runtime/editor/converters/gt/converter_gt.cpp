#include "runtime/editor/converters/gt/converter_gt.h"
#include "runtime/editor/converters/gt/source_sng.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_console.h"
#include "runtime/editor/converters/utils/sf2_interface.h"
#include "foundation/platform/iplatform.h"
#include "foundation/graphics/textfield.h"
#include "utils/c64file.h"
#include "libraries/ghc/fs_std.h"
#include "foundation/base/assert.h"

using namespace fs;

namespace Editor
{
	ConverterGT::ConverterGT()
	{
	}

	ConverterGT::~ConverterGT()
	{
	}

	const std::string ConverterGT::GetName() const { return "Goat Tracker converter"; }

	bool ConverterGT::CanConvert(const void* inData, unsigned int inDataSize) const
	{
		// Assert that there's is some data in the first place
		FOUNDATION_ASSERT(inData != nullptr);
		FOUNDATION_ASSERT(inDataSize > 0);

		if (inDataSize >= 4)
		{
			if (memcmp("GTS3", inData, 4) == 0)
				return true;
			if (memcmp("GTS4", inData, 4) == 0)
				return true;
			if (memcmp("GTS5", inData, 4) == 0)
				return true;
		}

		return false;
	}


	bool ConverterGT::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		return false;
	}


	bool ConverterGT::Update() 
	{
		FOUNDATION_ASSERT(GetState() != State::Uninitialized);

		if (GetState() == State::Initialized)
		{
			m_State = State::Completed;

			SF2::Interface sf2(m_Platform, *m_Console);
			const path driver_path = m_Platform->Storage_GetDriversHomePath();
			const path driver_path_and_filename = driver_path / "sf2driver11_04.prg";
			const bool driver_loaded = sf2.LoadFile(driver_path_and_filename.string());

			if (!driver_loaded)
				sf2.GetCout() << "\nFailed to load driver: " << driver_path_and_filename.string();
			else
			{
				Converter::SourceSng converter(&sf2, static_cast<unsigned char*>(m_Data));

				if (converter.Convert(0))
					m_Result = sf2.GetResult();
				else
					sf2.GetCout() << "\nConversion failed!";
			}
		}

		return true;
	}


	void ConverterGT::Setup()
	{
		const auto& dimensions = m_TextField->GetDimensions();
		m_Console = std::make_shared<ComponentConsole>(0, 0, nullptr, m_TextField, 1, 2, dimensions.m_Width - 2, dimensions.m_Height - 5);
		m_ComponentsManager->AddComponent(m_Console);
	}
}
