#include "platform_sdl_linux.h"
#include "mutex_sdl.h"

#ifdef _SF2_LINUX
#include "libraries/ghc/fs_std.h"
#include <assert.h>
#include <system_error>
#include <pwd.h>
#include <libgen.h>
#include <unistd.h>

using namespace fs;

namespace Foundation
{
	PlatformSDLLinux::PlatformSDLLinux()
        : PlatformSDL("linux")
	{
        std::error_code error_code;
        char buffer[PATH_MAX];

        // Standard mount points for external Drives
        m_LogicalDrivesList.push_back("/mnt");
        m_LogicalDrivesList.push_back("/Media");

        // Add the user's home folder
        char *realHome = getpwuid(getuid())->pw_dir;

        if(realHome != nullptr)
        {
            m_RealHome = std::string(realHome);

            m_LogicalDrivesList.push_back(realHome);

            // Add some standard folders that user might want to use...
            m_LogicalDrivesList.push_back(std::string(realHome) + "/Desktop");
            m_LogicalDrivesList.push_back(std::string(realHome) + "/Music");
            m_LogicalDrivesList.push_back(std::string(realHome) + "/Documents");
        }

        // let's also add the current dir
        fs::path application_path = getcwd(buffer, PATH_MAX);
        m_LogicalDrivesList.push_back(std::string(buffer));

        if(!fs::is_directory(application_path))
            fs::current_path(application_path.remove_filename());
        else
            fs::current_path(application_path);//, error_code);
    }


	PlatformSDLLinux::~PlatformSDLLinux()
	{
	}

	//---------------------------------------------------------------------------------------

    unsigned int PlatformSDLLinux::Storage_GetLogicalDrivesCount() const
    {
        return static_cast<unsigned int>(m_LogicalDrivesList.size());
    }

    std::string PlatformSDLLinux::Storage_GetLogicalDriveName(unsigned int inLogicalDrive) const
    {
        assert(inLogicalDrive < m_LogicalDrivesList.size());
        return m_LogicalDrivesList[inLogicalDrive];
    }

    bool PlatformSDLLinux::Storage_SetCurrentPath(const std::string& inPath) const
    {
        std::error_code ec;
        fs::path previous_path = fs::current_path(ec);

        if (!ec)
        {
            // Set current path, if that fails, set the prevcious path again
            fs::current_path(inPath, ec);
            if (ec)
            {
                fs::current_path(previous_path, ec);
                return false;
            }

            // Try get the current path to verify that it works (which will pop up a dialog on macOS for instance, if there are special enforcements to consider. If getting the path fails, set the previous again.
            auto cp = fs::current_path(ec);

            fs::directory_iterator(cp, ec);
            if(!ec)
                return true;

            fs::current_path(previous_path, ec);
        }

        return false;
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
       return GetResourcePath("config");
    }


    std::string PlatformSDLLinux::Storage_GetDriversHomePath() const
    {
       return GetResourcePath("drivers");
    }


    std::string PlatformSDLLinux::Storage_GetOverlaysHomePath() const
    {
        return GetResourcePath("overlay");
    }


    std::string PlatformSDLLinux::Storage_GetColorSchemesHomePath() const
    {
       return GetResourcePath("color_schemes");
    }


    std::string PlatformSDLLinux::OS_ParsePath(const std::string& inPath) const
    {
        return inPath;
    }

    // https://stackoverflow.com/questions/23943239/how-to-get-path-to-current-exe-file-on-linux
    std::string PlatformSDLLinux::GetResourcePath(const std::string& relativePath) const {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string resourcePath = m_ApplicationPath;
        if (count != -1)
            resourcePath = std::string(dirname(result));
        return resourcePath + "/" + relativePath;
    }

}

#endif
