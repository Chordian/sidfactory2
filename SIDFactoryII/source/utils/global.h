#pragma once

namespace Foundation
{
	class IPlatform;
}

namespace Utility
{
	// Singleton for global configuration
	class Global
	{

	public:
		static Global& instance();
		Foundation::IPlatform* m_Platform;
		void deletePlatform();

	private:
		Global();
	};

}
