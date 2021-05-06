#pragma once

#include <string>
#include <vector>

namespace Utility
{
	class ConfigFile;
}

namespace Foundation
{
	class IPlatform;
	class Viewport;
}

namespace Editor
{
	class DriverInfo;

	class OverlayControl
	{
	public:
		OverlayControl(Foundation::Viewport* inViewport);

		void SetOverlayEnabled(bool inEnabled);
		bool GetOverlayEnabled() const;

		void Update(int inDeltaTicks);

		void OnChange(const DriverInfo& inDriverInfo);
		void OnWindowResized();

	private:
		void ReadConfigValues(const Utility::ConfigFile& inConfigFile);
		void EnumeratePlatformFiles(const Foundation::IPlatform& inPlatform);
		void LoadOverlay(bool inIsEditorOverlay, const std::string& inFilename);

		bool m_Enabled;
		bool m_OverlayEnabledState;
		bool m_IsFading;
		float m_FadeValue;

		Foundation::Viewport* m_Viewport;
		std::vector<std::string> m_OverlayFileList;

		int m_OverlayEditorClientOffsetX;
		int m_OverlayEditorClientOffsetY;
		int m_OverlayWidth;
		int m_OverlayHeight;
		int m_OverlayEditorImageX;
		int m_OverlayEditorImageY;
		int m_OverlayDriverImageX;
		int m_OverlayDriverImageY;
		int m_OverlayFadeDuration;
	};
}