#include "runtime/editor/overlay_control.h"
#include "runtime/editor/driver/driver_info.h"
#include "utils/configfile.h"
#include "utils/utilities.h"
#include "libraries/picopng/picopng.h"
#include "libraries/ghc/fs_std.h"
#include "foundation/platform/iplatform.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/types.h"
#include <algorithm>


namespace Editor
{
	using namespace fs;
	using namespace Utility;
	using namespace Utility::Config;

	OverlayControl::OverlayControl(const Utility::ConfigFile& inConfigFile, Foundation::Viewport* inViewport, const Foundation::IPlatform* inPlatform)
		: m_Enabled(false)
		, m_OverlayEnabledState(false)
		, m_Viewport(inViewport)
		, m_IsFading(true)
		, m_FadeValue(0.0f)
	{
		ReadConfigValues(inConfigFile);
		EnumeratePlatformFiles(inPlatform);
		path overlays_path = inPlatform->Storage_GetOverlaysHomePath();
		LoadOverlay(true, (overlays_path / (inPlatform->GetName() + "_editor.png")).string());
		SetOverlayEnabled(GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Show.Overlay", 0) != 0);
	}


	void OverlayControl::Update(int inDeltaTicks)
	{
		if (m_IsFading)
		{
			float fade_delta = m_OverlayFadeDuration > 0 ? (static_cast<float>(inDeltaTicks) / m_OverlayFadeDuration) : 1.0f;
			if (m_Enabled != m_OverlayEnabledState)
			{
				if (m_FadeValue > 0.0f)
					m_FadeValue = std::max<float>(m_FadeValue - fade_delta, 0.0f);
				else
				{
					const Foundation::Point editor_client_offset = { m_OverlayEditorClientOffsetX, m_OverlayEditorClientOffsetY };
					const Foundation::Point window_position = m_Viewport->GetWindowPosition();

					if (m_Enabled)
					{
						m_Viewport->SetWindowSize({ m_OverlayWidth, m_OverlayHeight });
						m_Viewport->SetWindowPosition(window_position - editor_client_offset);
					}
					else
					{
						m_Viewport->SetWindowSize({ m_Viewport->GetClientWidth(), m_Viewport->GetClientHeight() });
						m_Viewport->SetWindowPosition(window_position + editor_client_offset);
					}

					m_OverlayEnabledState = m_Enabled;
				}
			}
			else
			{
				if (m_FadeValue < 1.0f)
					m_FadeValue = std::min<float>(m_FadeValue + fade_delta, 1.0f);
				else
					m_IsFading = false;
			}

			m_Viewport->SetFadeValue(m_FadeValue);
		}
	}


	void OverlayControl::SetOverlayEnabled(bool inEnabled)
	{
		if (m_Enabled != inEnabled && !m_IsFading)
		{
			m_Enabled = inEnabled;
			m_IsFading = true;
		}
	}


	bool OverlayControl::GetOverlayEnabled() const
	{
		return m_Enabled;
	}


	void OverlayControl::OnChange(const DriverInfo& inDriverInfo)
	{
		if (inDriverInfo.IsValid())
		{
			const auto& descriptor = inDriverInfo.GetDescriptor();

			std::string version = std::to_string(descriptor.m_DriverVersionMajor) + "_" + std::to_string(descriptor.m_DriverVersionMinor / 10) + std::to_string(descriptor.m_DriverVersionMinor % 10);

			for (const std::string& filename : m_OverlayFileList)
			{
				if (filename.find(version) != std::string::npos)
				{
					LoadOverlay(false, filename);
					return;
				}
			}
		}

		// Load empty overlay
	}


	void OverlayControl::OnWindowResized()
	{
		if (m_Enabled)
		{
			const Foundation::Point editor_client_offset = { m_OverlayEditorClientOffsetX, m_OverlayEditorClientOffsetY };
			m_Viewport->SetClientPositionInWindow(editor_client_offset);
		}
		else
			m_Viewport->SetClientPositionInWindow({ 0, 0 });

		m_Viewport->ShowOverlay(m_Enabled);
	}



	void OverlayControl::ReadConfigValues(const Utility::ConfigFile& inConfigFile)
	{
		m_OverlayWidth = Utility::GetSingleConfigurationValue<Utility::Config::ConfigValueInt>(inConfigFile, "Overlay.Width", 0);
		m_OverlayHeight = Utility::GetSingleConfigurationValue<Utility::Config::ConfigValueInt>(inConfigFile, "Overlay.Height", 0);
		m_OverlayEditorClientOffsetX = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Editor.Position.X", 0);
		m_OverlayEditorClientOffsetY = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Editor.Position.Y", 0);
		m_OverlayFadeDuration = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Fade.Duration", 100);

		m_OverlayEditorImageX = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Editor.Image.X", 0);
		m_OverlayEditorImageY = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Editor.Image.Y", 0);
		m_OverlayDriverImageX = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Driver.Image.X", 0);
		m_OverlayDriverImageY = GetSingleConfigurationValue<ConfigValueInt>(inConfigFile, "Overlay.Driver.Image.Y", 0);
	}


	void OverlayControl::EnumeratePlatformFiles(const Foundation::IPlatform* inPlatform)
	{
		const std::string& platform_name = inPlatform->GetName();
		directory_iterator directory_iterator(inPlatform->Storage_GetOverlaysHomePath());

		for (auto& path : directory_iterator)
		{
			std::error_code error_code;

			if (is_regular_file(path, error_code))
			{
				if(path.path().filename().string().find(platform_name) != std::string::npos)
					m_OverlayFileList.push_back(path.path().string());
			}
		}

		// sort the list, so that versions of drivers are ordered. This will be usefull to display a driver overlay of a lesser minor version!
	}


	void OverlayControl::LoadOverlay(bool inIsEditorOverlay, const std::string& inFilename)
	{
		using namespace Foundation;

		void* file_buffer;
		long file_size;

		if (Utility::ReadFile(inFilename, 0, &file_buffer, file_size))
		{
			std::vector<unsigned char> decoded_image;
			unsigned long decoded_image_width;
			unsigned long decoded_image_height;

			if (PicoPNG::decodePNG(decoded_image, decoded_image_width, decoded_image_height, static_cast<const unsigned char*>(file_buffer), file_size, true) == 0)
			{
				unsigned char* data = new unsigned char[decoded_image.size()];
				
				for (unsigned int i = 0; i < decoded_image.size(); ++i)
					data[i] = decoded_image[i];
			
				Rect rect = 
				{
					inIsEditorOverlay ? m_OverlayEditorImageX : m_OverlayDriverImageX,
					inIsEditorOverlay ? m_OverlayEditorImageY : m_OverlayDriverImageY,
					static_cast<int>(decoded_image_width),
					static_cast<int>(decoded_image_height)
				};

				m_Viewport->SetOverlayPNG(inIsEditorOverlay ? 0 : 1, static_cast<void*>(data), rect);
				delete[] data;
			}

			delete[] static_cast<unsigned char*>(file_buffer);
		}
	}
}