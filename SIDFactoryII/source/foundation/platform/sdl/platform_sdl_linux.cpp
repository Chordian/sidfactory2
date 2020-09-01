#include "platform_sdl_linux.h"
#include "mutex_sdl.h"

#ifdef _SF2_LINUX
#include "libraries/ghc/fs_std.h"
#include <assert.h>
#include <system_error>

using namespace fs;

namespace Foundation
{
	PlatformSDLLinux::PlatformSDLLinux()
        : PlatformSDL("linux")
	{
    }


	PlatformSDLLinux::~PlatformSDLLinux()
	{
	}

	//---------------------------------------------------------------------------------------

    unsigned int PlatformSDLLinux::Storage_GetLogicalDrivesCount() const
    {
        return 0;
    }


    std::string PlatformSDLLinux::Storage_GetLogicalDriveName(unsigned int inLogicalDrive) const
    {
        return "";
    }


    bool PlatformSDLLinux::Storage_SetCurrentPath(const std::string& inPath) const
    {
        std::error_code ec;
        fs::current_path(inPath, ec);
        if (ec)
            return false;

        return true;
    }


    bool PlatformSDLLinux::Storage_IsSystemFile(const std::string& inPath) const
    {
        fs::path path(inPath);

        // Remove any filename that starts with a dot
        std::string filename = path.filename().string();
        if (filename != "." && filename != ".." && filename[0] == '.')
            return true;

        return false;
    }


    bool PlatformSDLLinux::Storage_DeleteFile(const std::string& inPath) const
    {
        fs::path path_and_filename = inPath;

        if (fs::is_regular_file(path_and_filename))
        {
            std::error_code ec;
            return fs::remove(path_and_filename, ec);
        }

        return false;
    }


    std::string PlatformSDLLinux::Storage_GetApplicationHomePath() const
    {
         return m_RealHome;      // FIXME: not used, delete?
    }


    std::string PlatformSDLLinux::Storage_GetHomePath() const
    {
        return m_RealHome;      // This should be the user default 
    }


    std::string PlatformSDLLinux::Storage_GetConfigHomePath() const
    {
        return m_RealHome;      // This should be the user default 
    }


    std::string PlatformSDLLinux::Storage_GetDriversHomePath() const
    {
        return m_RealHome;
    }

    
    std::string PlatformSDLLinux::Storage_GetOverlaysHomePath() const
    {
        return m_RealHome;
    }


    std::string PlatformSDLLinux::Storage_GetColorSchemesHomePath() const 
    {
        return m_RealHome;
    }


    std::string PlatformSDLLinux::OS_ParsePath(const std::string& inPath) const
    {
        return inPath;
    }
}

#endif
