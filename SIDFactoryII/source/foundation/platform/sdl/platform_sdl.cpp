#include "platform_sdl.h"
#include "mutex_sdl.h"

#include "foundation/base/assert.h"

#include "libraries/ghc/fs_std.h"
#include <system_error>

using namespace fs;

#ifdef CreateMutex
#define __UNDEF_CREATEMUTEX
#pragma push_macro("CreateMutex")
#undef CreateMutex
#endif

namespace Foundation
{
	PlatformSDL::PlatformSDL(const std::string& inName)
		: m_Name(inName)
	{
	}


	PlatformSDL::~PlatformSDL()
	{
	}

	//---------------------------------------------------------------------------------------

	const std::string& PlatformSDL::GetName() const
	{
		return m_Name;
	}

	std::shared_ptr<IMutex> PlatformSDL::CreateMutex()
	{
		return std::shared_ptr<IMutex>(new MutexSDL());
	}

	//---------------------------------------------------------------------------------------

}

#ifdef __UNDEF_CREATEMUTEX
#pragma pop_macro("CreateMutex")
#undef __UNDEF_CREATEMUTEX
#endif //__PUSHED_CREATEMUTEX
