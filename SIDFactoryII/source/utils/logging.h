#pragma once

#include <SDL.h>
#include <string>

namespace Utility
{

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