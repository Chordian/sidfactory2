#pragma once

#include "dialog_base.h"
#include "runtime/editor/datasources/datasource_memory_buffer.h"
#include "runtime/editor/components/component_text_input.h"
#include "runtime/editor/components/component_button.h"
#include "utils/sidpanning.h"

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class DialogSIDFileInfo : public DialogBase
	{
	public:
		// File format and stereo panning of a multi SID export
		struct ExportOptions
		{
			bool m_Version5 = false;
			Utility::SIDPanLayout m_PanLayout = Utility::SIDPanLayout::Standard;
			Utility::SIDPanMode m_PanMode = Utility::SIDPanMode::Direct;
		};

		DialogSIDFileInfo(std::function<void(std::string, std::string, std::string)>&& inDone, std::function<void(void)>&& inCancel);

		// A tune with more than one SID adds the format, panning layout and panning mode rows
		DialogSIDFileInfo(
			unsigned int inSIDCount,
			const ExportOptions& inExportOptions,
			std::function<void(std::string, std::string, std::string, ExportOptions)>&& inDone,
			std::function<void(void)>&& inCancel);

		void Cancel() override;
		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse) override;

	protected:
		virtual void ActivateInternal(Foundation::Viewport* inViewport) override;
		virtual void DeactivateInternal(Foundation::Viewport* inViewport) override;

		void OnDone();

		bool HasExportOptions() const;
		void UpdateExportOptionTexts();

		std::string ConvertToString(std::shared_ptr<DataSourceMemoryBuffer>& inMemoryBuffer);

	private:
		int m_Width;
		int m_Height;

		int m_MessageMaxLength;

		Foundation::TextField* m_TextField;

		std::function<void(std::string, std::string, std::string)> m_DoneFunction;
		std::function<void(std::string, std::string, std::string, ExportOptions)> m_DoneWithOptionsFunction;
		std::function<void(void)> m_CancelFunction;

		unsigned int m_SIDCount;
		ExportOptions m_ExportOptions;

		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBufferTitle;
		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBufferAuthor;
		std::shared_ptr<DataSourceMemoryBuffer> m_TextDataBufferCopyright;
		std::shared_ptr<ComponentTextInput> m_TextInputComponentTitle;
		std::shared_ptr<ComponentTextInput> m_TextInputComponentAuthor;
		std::shared_ptr<ComponentTextInput> m_TextInputComponentCopyright;
		std::shared_ptr<ComponentButton> m_ComponentButtonFormat;
		std::shared_ptr<ComponentButton> m_ComponentButtonPanLayout;
		std::shared_ptr<ComponentButton> m_ComponentButtonPanMode;
		std::shared_ptr<ComponentButton> m_ComponentButtonOk;
	};
}