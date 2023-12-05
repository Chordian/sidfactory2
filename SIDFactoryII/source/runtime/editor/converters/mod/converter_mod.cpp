#include "runtime/editor/converters/mod/converter_mod.h"
#include "runtime/editor/converters/mod/source_mod.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/components/component_console.h"
#include "runtime/editor/converters/utils/sf2_interface.h"
#include "runtime/editor/dialog/dialog_selection_list.h"
#include "foundation/platform/iplatform.h"
#include "foundation/graphics/textfield.h"
#include "utils/usercolors.h"

#include "utils/c64file.h"
#include "libraries/ghc/fs_std.h"
#include "foundation/base/assert.h"
#include <memory>

using namespace fs;
using namespace Utility;

namespace Editor
{
	ConverterMod::ConverterMod()
		: m_IgnoreChannel(0)
	{
	}

	ConverterMod::~ConverterMod()
	{
	}

	const std::string ConverterMod::GetName() const { return "MOD converter"; }

	bool ConverterMod::CanConvert(const void* inData, unsigned int inDataSize) const
	{
		// Assert that there's is some data in the first place
		FOUNDATION_ASSERT(inData != nullptr);
		FOUNDATION_ASSERT(inDataSize > 0);

		if (inDataSize >= 1084)
		{
			const char* data = static_cast<const char*>(inData);
			if (memcmp("M.K.", &data[1080], 4) == 0)
				return true;
			if (memcmp("FLT", &data[1080], 3) == 0)
				return true;
		}

		return false;
	}


	bool ConverterMod::ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers)
	{
		return false;
	}


	bool ConverterMod::Update() 
	{
		FOUNDATION_ASSERT(m_State != State::Uninitialized);

		switch (m_State)
		{
		case State::Initialized:
			{		
				const path driver_path = m_Platform->Storage_GetDriversHomePath();
				const path driver_path_and_filename = driver_path / "sf2driver11_05.prg";
				const bool driver_loaded = m_ConversionUtility->LoadFile(driver_path_and_filename.string());

				if (!driver_loaded)
				{
					m_ConversionUtility->GetCout() << "\nFailed to load driver: " << driver_path_and_filename.string();
					m_State = State::Completed;
				}
				else
				{
					m_Converter = std::make_shared<Converter::SourceMod>(&(*m_ConversionUtility), static_cast<unsigned char*>(m_Data));

					auto on_select = [&](const unsigned int inSelection)
					{
						switch (inSelection)
						{
						case 0:
						case 1:
						case 2:
						case 3:
							m_IgnoreChannel = inSelection + 1;
							break;
						default:
							m_IgnoreChannel = 0;
							break;
						}

						m_State = State::Convert;
					};

					auto on_cancel = [&]()
					{
						m_ConversionUtility->GetCout() << "\nConversion cancelled!";
						m_State = State::Completed;
					};

					std::vector<std::string> selections = { "Channel 1", "Channel 2", "Channel 3", "Channel 4" };

					m_ComponentsManager->StartDialog(
						std::make_shared<DialogSelectionList>
						(
							60,
							7,
							0,
							"Select channel to ignore!",
							selections,
							on_select, 
							on_cancel
						)
					);

					m_State = State::Input;
				}
			}
			break;
		case State::Input:
			break;
		case State::Convert:
			{
				if (m_Converter->Convert(static_cast<int>(m_IgnoreChannel), 0))
					m_Result = m_ConversionUtility->GetResult();
				else
					m_ConversionUtility->GetCout() << "\nConversion failed!";

				m_State = State::Completed;
			}
			break;
		default:
			break;
		}

		return true;
	}


	void ConverterMod::Setup()
	{
		const auto& dimensions = m_TextField->GetDimensions();
		m_Console = std::make_shared<ComponentConsole>(0, 0, nullptr, m_TextField, 1, 2, dimensions.m_Width - 2, dimensions.m_Height - 5);
		m_ComponentsManager->AddComponent(m_Console);

		m_ConversionUtility = std::make_shared<SF2::Interface>(m_Platform, *m_Console);
	}
}
