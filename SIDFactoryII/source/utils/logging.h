#pragma once

#include <SDL.h>
#include <string>

namespace Utility
{

	class Logging
	{

	public:
		static Logging& instance();
		void Info(const std::string& message);
		void Warning(const std::string& message);
		void Error(const std::string& message);
		~Logging();

	protected:
		Logging();
	};

}