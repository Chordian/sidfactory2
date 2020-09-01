#pragma once

#include "sidproxydefines.h"
#include <stdio.h>
#include <vector>
#include <string>

namespace reSIDfp
{
	class SID;
}

namespace Emulation
{
	class SIDProxy
	{
	public:
		SIDProxy(const SIDConfiguration& sConfiguration);
		~SIDProxy();

		void SetEnvironment(SIDEnvironment eEnvironment);
		void SetModel(SIDModel eModel);
		void SetSampleMethod(SIDSampleMethod eSampleMethod);
		void SetSampleFrequency(int nSampleFrequency);

		SIDEnvironment GetEnvironment() const;
		SIDModel GetModel() const;
		SIDSampleMethod GetSampleMethod() const;
		int GetSampleFrequency() const;

		void SetConfiguration(const SIDConfiguration& sConfiguration);
		void ApplySettings();

		void StartRecordToFile(const std::string& inFileName);
		void StopRecordToFile();
		bool IsRecordingToFile() const;

		// Runtime
		void Reset();

		int Clock(int& nDeltaCycles, short* pBuffer, int nBufferSize);
		void Write(unsigned char ucReg, unsigned char ucValue);

	private:
		std::string m_FileName;
		std::vector<short> m_FileOutput;

		SIDConfiguration m_sConfiguration;

		reSIDfp::SID* m_pSID;

		int m_SampleCounter;
	};
}