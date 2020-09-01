#pragma once

#include <memory>
#include <string>

namespace Foundation
{
	// Forward declaration
	class IMutex;

	class IPlatform
	{
	protected:
		IPlatform() { }

	public: 
		virtual ~IPlatform() { }

		virtual std::shared_ptr<IMutex> CreateMutex() = 0;

		virtual const std::string& GetName() const = 0;
        
		virtual unsigned int Storage_GetLogicalDrivesCount() const = 0;
		virtual std::string Storage_GetLogicalDriveName(unsigned int inLogicalDrive) const = 0;

		virtual bool Storage_SetCurrentPath(const std::string& inPath) const = 0;
		virtual bool Storage_IsSystemFile(const std::string& inPath) const = 0;
		virtual bool Storage_DeleteFile(const std::string& inPath) const = 0;

		// Get the path to the folder that was set on startup of the application
		virtual std::string Storage_GetApplicationHomePath() const = 0;

		// Get the locigal home path (could be personal user space, or similar)
		virtual std::string Storage_GetHomePath() const = 0;

		// Get the path to the folder holding the main config file
		virtual std::string Storage_GetConfigHomePath() const = 0;

		// Get the path to the folder holding drivers
		virtual std::string Storage_GetDriversHomePath() const = 0;

		// Get the path to the folder holding overlay images
		virtual std::string Storage_GetOverlaysHomePath() const = 0;

		// Get the path to the folder holding color schemes
		virtual std::string Storage_GetColorSchemesHomePath() const = 0;

		// Parse path string for OS specific path aliases
		virtual std::string OS_ParsePath(const std::string& inPath) const = 0;
	};
}
