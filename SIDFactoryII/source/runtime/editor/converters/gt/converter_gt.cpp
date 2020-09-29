#include "runtime/editor/converters/gt/converter_gt.h"
#include "runtime/editor/converters/gt/source_sng.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/components_manager.h"
#include "runtime/editor/converters/utils/sf2_interface.h"
#include "foundation/platform/iplatform.h"
#include "utils/c64file.h"
#include "libraries/ghc/fs_std.h"
#include <assert.h>

using namespace fs;

namespace Editor
{
	ConverterGT::ConverterGT()
	{
	}

	ConverterGT::~ConverterGT()
	{
	}

	bool ConverterGT::Convert
	(
		void* inData,
		unsigned int inDataSize,
		Foundation::IPlatform* inPlatform,
		ComponentsManager& inComponentsManager,
		std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
	) 
	{
		// Assert that there's is some data in the first place
		assert(inData != nullptr);
		assert(inDataSize > 0);

		// Detect if the converter can presumably convert the data
		const bool can_convert_this = CanConvertInput(inData, inDataSize);

		if (can_convert_this)
		{
			// Store the success action
			m_SuccessAction = inSuccessAction;

			SF2::Interface sf2(inPlatform);
			const path driver_path = inPlatform->Storage_GetDriversHomePath();
			const path driver_path_and_filename = driver_path / "sf2driver11_02.prg";
			bool driver_loaded = sf2.LoadFile(driver_path_and_filename.string());

			Converter::SourceSng converter(&sf2, static_cast<unsigned char*>(inData));
			if (converter.Convert(0))
			{
				m_Result = sf2.GetResult();
				
				inComponentsManager.StartDialog(std::make_shared<DialogMessage>("GT Converter", "Conversion complete!", 80, true, [&]()
					{
						m_SuccessAction(m_Result);
					}
				));
			}
			else
			{
				inComponentsManager.StartDialog(std::make_shared<DialogMessage>("GT Converter", "Error! \n" + converter.GetErrorMessage(), 80, true, [&]()
					{
						m_SuccessAction(std::shared_ptr<Utility::C64File>());
					}
				));
			}

			// Show a dialog, to test the flow!

			// Return true, to indicate that the convertion 
			return true;
		}

		// Return false, if the converter cannot convert the type of data parsed in!
		return false;
	}


	bool ConverterGT::CanConvertInput(void* inData, unsigned int inDataSize) const
	{
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
}