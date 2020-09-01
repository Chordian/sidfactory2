#pragma once

#include "foundation/platform/sdl/platform_sdl.h"
#include <vector>
#include <string>

#ifndef _SF2_WINDOWS
#ifndef _SF2_LINUX
#include <limits.h>

namespace Foundation
{
	class PlatformSDLMacOS final : public PlatformSDL
	{
	public:
		explicit PlatformSDLMacOS();
		virtual ~PlatformSDLMacOS();

		unsigned int Storage_GetLogicalDrivesCount() const override;
		std::string Storage_GetLogicalDriveName(unsigned int inLogicalDrive) const override;

		bool Storage_SetCurrentPath(const std::string& inPath) const override;
		bool Storage_IsSystemFile(const std::string& inPath) const override;
		bool Storage_DeleteFile(const std::string& inPath) const override;

		virtual std::string Storage_GetApplicationHomePath() const override;
		virtual std::string Storage_GetHomePath() const override;
		virtual std::string Storage_GetConfigHomePath() const override;
		virtual std::string Storage_GetDriversHomePath() const override;
		virtual std::string Storage_GetOverlaysHomePath() const override;
		virtual std::string Storage_GetColorSchemesHomePath() const override;
		virtual std::string OS_ParsePath(const std::string& inPath) const override;

	private:
		unsigned long m_LogicalDrivesMask;
		std::vector<std::string> m_LogicalDrivesList;
        
        char m_ApplicationPath[PATH_MAX];
        std::string m_RealHome;
        std::string GetResourcePath(const std::string& relativePath) const;
	};
}
#endif
#endif
