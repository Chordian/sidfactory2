#pragma once

#include <memory>
#include <functional>

namespace Foundation
{
	class IPlatform;
}

namespace Utility
{
	class C64File;
}

namespace Editor
{
	class ComponentsManager;
	class ConverterBase
	{
	public:
		enum class State
		{
			Uninitialized,
			Initialized,
			Input,
			Completed
		};

	protected:
		ConverterBase();
	
	public:
		virtual ~ConverterBase();

		void Activate
		(
			void* inData,
			unsigned int inDataSize,
			Foundation::IPlatform* inPlatform,
			ComponentsManager* inComponentsManager
		);

		State GetState() const;
		std::shared_ptr<Utility::C64File> GetResult();


		virtual bool CanConvert(const void* inData, unsigned int inDataSize) const = 0;
		virtual bool Update
		(
		) = 0;

	protected:
		State m_State;

		void* m_Data;
		unsigned int m_DataSize;
		Foundation::IPlatform* m_Platform;
		ComponentsManager* m_ComponentsManager;

		std::shared_ptr<Utility::C64File> m_Result;
	};
}