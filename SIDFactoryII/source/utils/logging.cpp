#include "logging.h"

namespace Utility
{

	Logging& Logging::instance()
	{
		static Logging instance;
		return instance;
	}

	void Logging::Info(const std::string& message)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, message.c_str());
	}

	void Logging::Warning(const std::string& message)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, message.c_str());
	}

	void Logging::Error(const std::string& message)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, message.c_str());
	}

}