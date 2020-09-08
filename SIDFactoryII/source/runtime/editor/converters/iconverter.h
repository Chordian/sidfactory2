#pragma once

#include <memory>
#include <functional>

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
		virtual ~IConverter() 
		{
		}


		virtual bool Convert
		(
			void* inData, 
			unsigned int inDataSize, 
			ComponentsManager& inComponentsManager,
			std::function<void(std::shared_ptr<Utility::C64File>)> inSuccessAction
		) = 0;
	};
}