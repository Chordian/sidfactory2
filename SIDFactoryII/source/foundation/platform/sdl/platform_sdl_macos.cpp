#include "platform_sdl_macos.h"
#include "mutex_sdl.h"

#ifndef _SF2_WINDOWS
#ifndef _SF2_LINUX
#include "libraries/ghc/fs_std.h"
#include <assert.h>
#include <mach-o/dyld.h>
#include <system_error>
#include <pwd.h>
#include <ApplicationServices/ApplicationServices.h>

using namespace fs;

namespace Foundation
{
	PlatformSDLMacOS::PlatformSDLMacOS()
        : PlatformSDL("macos")
	{
        std::error_code error_code;

        uint32_t buffer_size = PATH_MAX;
        _NSGetExecutablePath(m_ApplicationPath, &buffer_size);

        m_LogicalDrivesList.push_back("/Volumes");

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

        path application_path = m_ApplicationPath;

        if(!fs::is_directory(application_path))
            current_path(application_path.remove_filename());
        else
            current_path(application_path);//, error_code);
    }


	PlatformSDLMacOS::~PlatformSDLMacOS()
	{

	}

	//---------------------------------------------------------------------------------------

    unsigned int PlatformSDLMacOS::Storage_GetLogicalDrivesCount() const
    {
        return static_cast<unsigned int>(m_LogicalDrivesList.size());
    }


    std::string PlatformSDLMacOS::Storage_GetLogicalDriveName(unsigned int inLogicalDrive) const
    {
        assert(inLogicalDrive < m_LogicalDrivesList.size());
        return m_LogicalDrivesList[inLogicalDrive];
    }


    bool PlatformSDLMacOS::Storage_SetCurrentPath(const std::string& inPath) const
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
            if (!ec)
                return true;

            fs::current_path(previous_path, ec);
        }

        return false;
    }


    bool PlatformSDLMacOS::Storage_IsSystemFile(const std::string& inPath) const
    {
        fs::path path(inPath);

        // Remove any filename that starts with a dot
        std::string filename = path.filename().string();
        if (filename != "." && filename != ".." && filename[0] == '.')
            return true;

        return false;
    }


    bool PlatformSDLMacOS::Storage_DeleteFile(const std::string& inPath) const
    {
        fs::path path_and_filename = inPath;

        if (fs::is_regular_file(path_and_filename))
        {
            std::error_code ec;
            return fs::remove(path_and_filename, ec);
        }

        return false;
    }


    std::string PlatformSDLMacOS::Storage_GetApplicationHomePath() const
    {
         return m_RealHome;      // FIXME: not used, delete?
    }


    std::string PlatformSDLMacOS::Storage_GetHomePath() const
    {
        return m_RealHome;      // This should be the user default
    }


    std::string PlatformSDLMacOS::Storage_GetConfigHomePath() const
    {
        return GetResourcePath("config");
    }


    std::string PlatformSDLMacOS::Storage_GetDriversHomePath() const
    {
        return GetResourcePath("drivers");
    }


    std::string PlatformSDLMacOS::Storage_GetOverlaysHomePath() const
    {
        return GetResourcePath("overlay");
    }


    std::string PlatformSDLMacOS::Storage_GetColorSchemesHomePath() const
    {
        return GetResourcePath("color_schemes");
    }


    // get path to a file or folder in the App bundle's Resources folder.
    std::string PlatformSDLMacOS::GetResourcePath(const std::string& relativePath) const
    {
        // https://stackoverflow.com/questions/24165681/accessing-files-in-resources-folder-in-mac-osx-app-bundle
        // TODO: refactor out string conversions for reuse
        // TODO: review encoding
        CFStringRef relativePathCF = CFStringCreateWithCString(kCFAllocatorDefault, relativePath.c_str(), kCFStringEncodingUTF8);
        CFURLRef appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), relativePathCF, NULL, NULL);

        if (appUrlRef != nullptr)
        {
            CFStringRef filePathRef = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
            const char* filePath = CFStringGetCStringPtr(filePathRef, kCFStringEncodingUTF8);
            return std::string(filePath) + '/';
        }

        return std::string();
    }


    std::string PlatformSDLMacOS::OS_ParsePath(const std::string& inPath) const
    {
        return inPath;
    }
}

#endif
#endif
