#include "runtime/editor/converters/cc/converter_cc.h"
#include "runtime/editor/converters/cc/source_ct.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_console.h"
#include "runtime/editor/converters/utils/sf2_interface.h"
#include "foundation/platform/iplatform.h"
#include "foundation/graphics/textfield.h"
#include "utils/c64file.h"
#include "libraries/ghc/fs_std.h"
#include <assert.h>

using namespace fs;

namespace Editor
{
	ConverterCC::ConverterCC()
	{
	}

	ConverterCC::~ConverterCC()
	{
	}


	bool ConverterCC::CanConvert(const void* inData, unsigned int inDataSize) const
	{
		// Assert that there's is some data in the first place
		assert(inData != nullptr);
		assert(inDataSize > 0);

		if (inDataSize >= 3)
		{
			if (memcmp("CC2", inData, 3) == 0)
				return true;
		}

		return false;
	}


	bool ConverterCC::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		return false;
	}


	bool ConverterCC::Update() 
	{
		assert(GetState() != State::Uninitialized);

		if (GetState() == State::Initialized)
		{
			m_State = State::Completed;

			SF2::Interface sf2(m_Platform, *m_Console);
			const path driver_path = m_Platform->Storage_GetDriversHomePath();
			const path driver_path_and_filename = driver_path / "sf2driver11_02.prg";
			bool driver_loaded = sf2.LoadFile(driver_path_and_filename.string());

			Converter::SourceCt converter(&sf2, static_cast<unsigned char*>(m_Data), static_cast<long>(m_DataSize));

			if (converter.CanConvert() && converter.Convert(0))
				m_Result = sf2.GetResult();
		}

		return true;
	}


	void ConverterCC::Setup()
	{
		const auto& dimensions = m_TextField->GetDimensions();
		m_Console = std::make_shared<ComponentConsole>(0, 0, nullptr, m_TextField, 1, 2, dimensions.m_Width - 2, dimensions.m_Height - 5);
		m_ComponentsManager->AddComponent(m_Console);
	}
}