#pragma once

#include <SDL_keycode.h>
#include <memory>
#include <functional>
#include <string>

namespace Foundation
{
	class IPlatform;
	class TextField;
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
			Convert,
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
			Foundation::TextField* inTextField,
			ComponentsManager* inComponentsManager
		);

		State GetState() const;
		std::shared_ptr<Utility::C64File> GetResult();

		virtual const std::string GetName() const = 0;
		virtual bool CanConvert(const void* inData, unsigned int inDataSize) const = 0;
		virtual bool ConsumeKeyEvent(SDL_Keycode inKeyEvent, unsigned int inModifiers) = 0;
		virtual bool Update() = 0;

	protected:
		virtual void Setup() = 0;

		State m_State;

		void* m_Data;
		unsigned int m_DataSize;
		Foundation::IPlatform* m_Platform;
		Foundation::TextField* m_TextField;
		ComponentsManager* m_ComponentsManager;

		std::shared_ptr<Utility::C64File> m_Result;
	};
}