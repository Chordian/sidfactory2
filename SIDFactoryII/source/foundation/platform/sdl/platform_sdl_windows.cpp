#include "platform_sdl_windows.h"
#include "mutex_sdl.h"

#ifdef _SF2_WINDOWS
#include "libraries/ghc/fs_std.h"
#include <windows.h>
#include <assert.h>

namespace Foundation
{
	PlatformSDLWindows::PlatformSDLWindows()
		: PlatformSDL("windows")
	{
		// Get current path
		m_ApplicationHomePath = (fs::current_path() / "").string();

		// Enumerate logic storage devices
		m_LogicalDrivesMask = GetLogicalDrives();

		for (char i = 0; i <= 'z' - 'a'; ++i)
		{
			if ((m_LogicalDrivesMask & (1 << i)) != 0)
			{
				std::string driver_letter = std::string(1, 'A' + i) + ":\\";
				m_LogicalDrivesList.push_back(driver_letter);
			}
		}
	}


	PlatformSDLWindows::~PlatformSDLWindows()
	{

	}

	//---------------------------------------------------------------------------------------
    
	unsigned int PlatformSDLWindows::Storage_GetLogicalDrivesCount() const
	{
		return static_cast<unsigned int>(m_LogicalDrivesList.size());
	}


	std::string PlatformSDLWindows::Storage_GetLogicalDriveName(unsigned int inLogicalDrive) const
	{
		assert(inLogicalDrive < m_LogicalDrivesList.size());
		return m_LogicalDrivesList[inLogicalDrive];
	}


    bool PlatformSDLWindows::Storage_SetCurrentPath(const std::string& inPath) const
    {
        std::error_code ec;
        fs::path previous_path = fs::current_path(ec);

        if (!ec)
        {
            // Set current path, if that fails, set the prevcious path again
            fs::current_path(inPath, ec);
            if (!ec)
				return true;
			
            fs::current_path(previous_path, ec);
        }

		return false;
    }


	bool PlatformSDLWindows::Storage_IsSystemFile(const std::string& inPath) const
	{
		DWORD file_attributes = GetFileAttributes(inPath.c_str());

		if ((file_attributes & FILE_ATTRIBUTE_HIDDEN) != 0)
			return true;

		fs::path path(inPath);

		// Remove any filename that starts with a dot
		std::string filename = path.filename().string();
		if (filename != "." && filename != ".." && filename[0] == '.')
			return true;

		return false;
	}


	bool PlatformSDLWindows::Storage_DeleteFile(const std::string& inPath) const
	{
		fs::path path_and_filename = inPath;

		if (fs::is_regular_file(path_and_filename))
		{
			std::error_code ec;
			return fs::remove(path_and_filename, ec);
		}

		return false;
	}


	std::string PlatformSDLWindows::Storage_GetApplicationHomePath() const
	{
		return m_ApplicationHomePath;
	}


	std::string PlatformSDLWindows::Storage_GetHomePath() const
	{
		return m_ApplicationHomePath;
	}


	std::string PlatformSDLWindows::Storage_GetConfigHomePath() const
	{
		return m_ApplicationHomePath;
	}


	std::string PlatformSDLWindows::Storage_GetDriversHomePath() const
	{
		fs::path drivers_path = fs::path(m_ApplicationHomePath) / "drivers" / "";
		return drivers_path.string();
	}


	std::string PlatformSDLWindows::Storage_GetOverlaysHomePath() const
	{
		fs::path overlays_path = fs::path(m_ApplicationHomePath) / "overlay" / "";
		return overlays_path.string();
	}


	std::string PlatformSDLWindows::Storage_GetColorSchemesHomePath() const
	{
		fs::path color_schemes_path = fs::path(m_ApplicationHomePath) / "color_schemes" / "";
		return color_schemes_path.string();
	}


	std::string PlatformSDLWindows::OS_ParsePath(const std::string& inPath) const
	{
		return inPath;
	}
}
#endif 

