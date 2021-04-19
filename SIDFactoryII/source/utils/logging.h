#pragma once

#include <SDL.h>
#include <string>

namespace Utility
{

	// Singleton used for logging diagnostic messages.
  // N.B. use only from the main thread!
	class Logging
	{

	public:
		static Logging& instance();
		void Info(const char* format, ...);
		void Warning(const char* format, ...);
		void Error(const char* format, ...);

	protected:
		Logging();
	};

}