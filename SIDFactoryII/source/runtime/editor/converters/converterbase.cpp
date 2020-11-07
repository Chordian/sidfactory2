#include "runtime/editor/converters/converterbase.h"
#include "foundation/base/assert.h"

namespace Editor
{
	ConverterBase::ConverterBase()
		: m_State(State::Uninitialized)
		, m_Platform(nullptr)
		, m_ComponentsManager(nullptr)
		, m_Result(nullptr)
	{ 
	}
	

	ConverterBase::~ConverterBase()
	{
	}


	void ConverterBase::Activate
	(
		void* inData,
		unsigned int inDataSize,
		Foundation::IPlatform* inPlatform,
		Foundation::TextField* inTextField,
		ComponentsManager* inComponentsManager
	)
	{
		FOUNDATION_ASSERT(GetState() == State::Uninitialized);

		m_Data = inData;
		m_DataSize = inDataSize;
		m_Platform = inPlatform;
		m_TextField = inTextField;
		m_ComponentsManager = inComponentsManager;

		m_State = State::Initialized;

		Setup();
	}


	std::shared_ptr<Utility::C64File> ConverterBase::GetResult()
	{
		FOUNDATION_ASSERT(m_State == State::Completed);

		return m_Result;
	}


	ConverterBase::State ConverterBase::GetState() const
	{
		return m_State;
	}
}