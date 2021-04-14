#include "logging.h"

namespace Utility
{

	Logging& Logging::instance()
	{
		static Logging instance;
		return instance;
	}

	Logging::Logging()
	{
	}

	void Logging::Info(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, format, argptr);
		va_end(argptr);
	}

	void Logging::Warning(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, format, argptr);
		va_end(argptr);
	}

	void Logging::Error(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, format, argptr);
		va_end(argptr);
	}

}