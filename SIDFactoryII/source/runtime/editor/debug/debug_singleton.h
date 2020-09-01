#pragma once

class DebugSingleton final
{
private:
	DebugSingleton()
		: m_HasDebugAddress(false)
		, m_DebugAddress(0) 
	{ 
	}

public:
	static DebugSingleton* GetInstance();

	void SetDebugAddress(unsigned short inDebugAddress);
	bool HasDebugAddress() const;
	unsigned short ConsumeDebugAddress();

private:
	bool m_HasDebugAddress;
	unsigned short m_DebugAddress;

	static DebugSingleton* m_Instance;
};

