#include "runtime/editor/converters/jch/converter_jch.h"
#include "runtime/editor/dialog/dialog_message.h"
#include "runtime/editor/components_manager.h"
#include "utils/c64file.h"
#include <assert.h>

namespace Editor
{
	ConverterJCH::ConverterJCH()
		: m_Data(nullptr)
	{
	}

	ConverterJCH::~ConverterJCH()
	{
		if (m_Data != nullptr)
			delete[] m_Data;
	}

	bool ConverterJCH::Convert
	(
		void* inData,
		unsigned int inDataSize,
		ComponentsManager& inComponentsManager,
		std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
	)
	{
		// Assert that there's is some data in the first place
		assert(inData != nullptr);
		assert(inDataSize > 0);

		// Detect if the converter can presumably convert the data
		const bool can_convert_this = IsFileValid(inData, inDataSize);

		if (can_convert_this)
		{
			// Make a copy of the data (this should usually only happen when the converter accepts the input data as something it can convert!)
			m_Data = new char[inDataSize];
			memcpy(m_Data, inData, inDataSize);

			// Store the success action
			m_SuccessAction = inSuccessAction;

			// Show a dialog, to test the flow!
			inComponentsManager.StartDialog(std::make_shared<DialogMessage>("NULL Converter", "You will now be converting this file to a null thing", 80, true, [&]()
				{
					// By now conversion should have completed, so we delete the local copy of the data
					if (m_Data != nullptr)
						delete[] m_Data;

					m_Data = nullptr;

					// Notify the success!
					m_SuccessAction(std::shared_ptr<Utility::C64File>());
				}
			));

			// Return true, to indicate that the convertion 
			return true;
		}

		// Return false, if the converter cannot convert the type of data parsed in!
		return false;
	}


	bool ConverterJCH::IsFileValid(void* inData, unsigned int inDataSize) const
	{
		if (inDataSize < 0x10000)
		{
			unsigned char* data = static_cast<unsigned char*>(inData);

			if (data[0] == 0x00 && data[1] == 0x00)
				return true;
		}

		return false;
	}
}