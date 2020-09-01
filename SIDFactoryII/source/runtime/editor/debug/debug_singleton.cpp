#include "debug_singleton.h"
#include <assert.h>

DebugSingleton* DebugSingleton::m_Instance = nullptr;

DebugSingleton* DebugSingleton::GetInstance()
{
	if (m_Instance == nullptr)
		m_Instance = new DebugSingleton();

	return m_Instance;
}


void DebugSingleton::SetDebugAddress(unsigned short inDebugAddress)
{
	m_HasDebugAddress = true;
	m_DebugAddress = inDebugAddress;
}


bool DebugSingleton::HasDebugAddress() const
{
	return m_HasDebugAddress;
}


unsigned short DebugSingleton::ConsumeDebugAddress()
{
	assert(m_HasDebugAddress);

	unsigned short return_address = m_DebugAddress;

	m_DebugAddress = 0;
	m_HasDebugAddress = false;

	return return_address;
}
