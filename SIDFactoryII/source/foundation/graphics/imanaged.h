#pragma once

namespace Foundation
{
	class IManaged
	{
	protected:
		IManaged() { }

	public:
		virtual ~IManaged() { }

		virtual void Begin() = 0;
		virtual void End() = 0;
	};
}