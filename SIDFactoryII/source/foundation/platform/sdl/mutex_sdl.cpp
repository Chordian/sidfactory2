#include "mutex_sdl.h"

namespace Foundation
{
	MutexSDL::MutexSDL()
	{
		m_Mutex = SDL_CreateMutex();
	}

	MutexSDL::~MutexSDL()
	{
		SDL_DestroyMutex(m_Mutex);
	}

	bool MutexSDL::TryLock()
	{
		return SDL_TryLockMutex(m_Mutex) == 0;
	}

	void MutexSDL::Lock()
	{
		SDL_LockMutex(m_Mutex);
	}

	void MutexSDL::Unlock()
	{
		SDL_UnlockMutex(m_Mutex);
	}
}