#pragma once

#include "foundation/graphics/color.h"
#include "foundation/input/mouse.h"
#include "runtime/editor/edit_state.h"
#include "runtime/editor/auxilarydata/auxilary_data_collection.h"
#include "runtime/editor/auxilarydata/auxilary_data_editing_preferences.h"
#include "runtime/editor/auxilarydata/auxilary_data_hardware_preferences.h"
#include "runtime/editor/screens/statusbar/status_bar.h"
#include "runtime/editor/driver/driver_state.h"
#include "runtime/execution/executionhandler.h"
#include <string>
#include <functional>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class StatusBarEdit : public StatusBar
	{
	public:
		StatusBarEdit
		(
			Foundation::TextField* inTextField,
			const EditState& inEditState,
			const DriverState& inDriverState,
			const AuxilaryDataCollection& inAuxilaryDataCollection,
			const Emulation::ExecutionHandler& inExecutionHandler,
			std::function<void(Foundation::Mouse::Button, int)> inOctaveMousePressCallback,
			std::function<void(Foundation::Mouse::Button, int)> inSharpFlatMousePressCallback,
			std::function<void(Foundation::Mouse::Button, int)> inSIDMousePressCallback,
			std::function<void(Foundation::Mouse::Button, int)> inOuputDevicePressCallback,
			std::function<void(Foundation::Mouse::Button, int)> inContextHighlightMousePressCallback,
			std::function<void(Foundation::Mouse::Button, int)> inFollowPlayerMousePressCallback
		);

		~StatusBarEdit();

		void SetDriverState(DriverState inDriverState);

	protected:
		void UpdateInternal(int inDeltaTick, bool inNeedUpdate) override;
		void ClearContents() override;
		void DrawText() override;

		std::shared_ptr<TextSection> m_TextSectionOctave;
		std::shared_ptr<TextSection> m_TextSectionSharpFlat;
		std::shared_ptr<TextSection> m_TextSectionSID;
		std::shared_ptr<TextSection> m_TextSectionContextHighlight;
		std::shared_ptr<TextSection> m_TextSectionFollowPlay;
		std::shared_ptr<TextSection> m_TextSectionOutputDevice;

		const EditState& m_EditState;
		const AuxilaryDataCollection& m_AuxilaryDataPlayMarkers;
		const DriverState& m_DriverState;
		const Emulation::ExecutionHandler& m_ExecutionHandler;

		EditState m_CachedEditState;
		DriverState m_CachedDriverState;

		AuxilaryDataEditingPreferences::NotationMode m_CachedNotationMode;
		AuxilaryDataHardwarePreferences::SIDModel m_CachedSIDModel;
		AuxilaryDataHardwarePreferences::Region m_CachedRegion;
		Emulation::ExecutionHandler::OutputDevice m_CachedOutputDevice; 
	};
}
