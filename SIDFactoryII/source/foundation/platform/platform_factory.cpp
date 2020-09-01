#include "platform_factory.h"

#include "foundation/platform/sdl/platform_sdl_windows.h"
#include "foundation/platform/sdl/platform_sdl_linux.h"
#include "foundation/platform/sdl/platform_sdl_macos.h"

namespace Foundation
{
	IPlatform* CreatePlatform()
	{
#ifdef _SF2_WINDOWS
		return new PlatformSDLWindows();
#elif _SF2_LINUX
		return new PlatformSDLLinux();
#else
		return new PlatformSDLMacOS();
#endif
	}
}