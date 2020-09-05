#pragma once

#include <memory>

namespace Utility
{
	class C64File;
}

namespace Editor
{
	class ComponentsManager;
	class IConverter
	{
	protected:
		IConverter() 
		{ 
		}
	
	public:
		~IConverter() 
		{
		}


		virtual std::shared_ptr<Utility::C64File> Convert(void* inData, unsigned int inDataSize, ComponentsManager& inComponentsManager) = 0;
	};
}