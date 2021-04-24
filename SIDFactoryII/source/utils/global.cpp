#include "global.h"
#include "foundation/platform/platform_factory.h"

using namespace Foundation;

namespace Utility
{

	Global& Global::instance()
	{
		static Global instance;
		return instance;
	}
	Global::Global()
		: m_Platform(Foundation::CreatePlatform()) {};

	void Global::deletePlatform()
	{
		delete m_Platform;
		m_Platform = nullptr;
	}

}
