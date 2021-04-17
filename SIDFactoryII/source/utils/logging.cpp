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
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, argptr);
		va_end(argptr);
	}

	void Logging::Warning(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, format, argptr);
		va_end(argptr);
	}

	void Logging::Error(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, format, argptr);
		va_end(argptr);
	}

}