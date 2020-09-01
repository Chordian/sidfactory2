#pragma once

#include "foundation/platform/iplatform.h"
#include <vector>
#include <string>

namespace Foundation
{
	class PlatformSDL : public IPlatform
	{
	protected: 
		PlatformSDL(const std::string& inName);

	public:
		virtual ~PlatformSDL();

		const std::string& GetName() const override;
		std::shared_ptr<IMutex> CreateMutex() override;

	private:
		std::string m_Name;
	};
}
