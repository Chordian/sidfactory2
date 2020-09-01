#pragma once

#include "foundation/platform/imutex.h"
#include "SDL.h"

namespace Foundation
{
	class MutexSDL final : public IMutex
	{
		friend class PlatformSDL;

	protected:
		MutexSDL();
	public:
		virtual ~MutexSDL();

		virtual bool TryLock() override;
		virtual void Lock() override;
		virtual void Unlock() override;

	private:
		SDL_mutex* m_Mutex;
	};
}