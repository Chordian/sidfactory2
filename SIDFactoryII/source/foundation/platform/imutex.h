#pragma once

namespace Foundation
{
	class IMutex
	{
	protected:
		IMutex() { }
	public:
		virtual ~IMutex() { }

		IMutex(const IMutex& inOther) = delete;
		IMutex(const IMutex&& inOther) = delete;

		virtual bool TryLock() = 0;
		virtual void Lock() = 0;
		virtual void Unlock() = 0;
	};
}