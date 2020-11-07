#include "sidproxy.h"
#include "runtime/environmentdefines.h"

#include "libraries/residfp/SID.h"

#include "foundation/base/assert.h"
#include <cmath>

namespace Emulation
{
	SIDProxy::SIDProxy(const SIDConfiguration& sConfiguration)
		: m_sConfiguration(sConfiguration)
		, m_SampleCounter(0)
	{
		// Create instance of reSid
		m_pSID = new reSIDfp::SID();

		// Apply settings
		ApplySettings();
	}

	SIDProxy::~SIDProxy()
	{
		delete m_pSID;
	}

	//------------------------------------------------------------------------------------------------------------

	void SIDProxy::SetEnvironment(SIDEnvironment eEnvironment)
	{
		m_sConfiguration.m_eEnvironment = eEnvironment;
	}


	void SIDProxy::SetModel(SIDModel eModel)
	{
		m_sConfiguration.m_eModel = eModel;
	}


	void SIDProxy::SetSampleMethod(SIDSampleMethod eSampleMethod)
	{
		m_sConfiguration.m_eSampleMethod = eSampleMethod;
	}


	void SIDProxy::SetSampleFrequency(int nSampleFrequency)
	{
		m_sConfiguration.m_nSampleFrequency = nSampleFrequency;
	}


	SIDEnvironment SIDProxy::GetEnvironment() const
	{
		return m_sConfiguration.m_eEnvironment;
	}


	SIDModel SIDProxy::GetModel() const
	{
		return m_sConfiguration.m_eModel;
	}


	SIDSampleMethod SIDProxy::GetSampleMethod() const
	{
		return m_sConfiguration.m_eSampleMethod;
	}


	int SIDProxy::GetSampleFrequency() const
	{
		return m_sConfiguration.m_nSampleFrequency;
	}

	//------------------------------------------------------------------------------------------------------------

	void SIDProxy::SetConfiguration(const SIDConfiguration& sConfiguration)
	{
		// Copy the configuration
		m_sConfiguration = sConfiguration;

		// Apply settings of configuration to sid emulation layer
		ApplySettings();
	}

	//------------------------------------------------------------------------------------------------------------

	void SIDProxy::ApplySettings()
	{
		using namespace reSIDfp;

		if (m_pSID != nullptr)
		{
			// Reset the sid
			m_pSID->reset();

			const double passband = 20000.0;

			m_pSID->setSamplingParameters
			(
				static_cast<double>(m_sConfiguration.m_eEnvironment == SID_ENVIRONMENT_PAL ? EMULATION_CYCLES_PER_SECOND_PAL : EMULATION_CYCLES_PER_SECOND_NTSC),
				m_sConfiguration.m_eSampleMethod != SID_SAMPLE_METHOD_RESAMPLE_INTERPOLATE ? SamplingMethod::DECIMATE : SamplingMethod::RESAMPLE,
				static_cast<double>(m_sConfiguration.m_nSampleFrequency),
				passband
			);

			m_pSID->setChipModel(m_sConfiguration.m_eModel == SID_MODEL_6581 ? ChipModel::MOS6581 : ChipModel::MOS8580);
		}
	}

	//------------------------------------------------------------------------------------------------------------

	void SIDProxy::StartRecordToFile(const std::string& inFileName)
	{
		FOUNDATION_ASSERT(m_FileName.empty());
		m_FileName = inFileName;
	}

	void SIDProxy::StopRecordToFile()
	{
		FOUNDATION_ASSERT(!m_FileName.empty());

		if (m_FileOutput.size() > 0)
		{
			FILE* file = fopen(m_FileName.c_str(), "wb");

			struct wave_header
			{
				char chunk_id[4] = { 'R', 'I', 'F', 'F' };
				int chunk_size;
				char format[4] = { 'W', 'A', 'V', 'E' }; 
				char sub_chunk1_id[4] = { 'f', 'm', 't', ' ' };
				int sub_chunk1_size;
				short audio_format = 1;
				short num_channels = 1;			//    Mono = 1, Stereo = 2, etc.
				int sample_rate = 44100;		//      8000, 44100, etc.
				int byte_rate = 88200;			//== SampleRate * NumChannels * BitsPerSample / 8
				short block_align = 2;			// == NumChannels * BitsPerSample / 8
				short bits_per_sample = 16;		//    8 bits = 8, 16 bits = 16, etc.
				char sub_chunk2_id[4] = { 'd', 'a', 't', 'a' };
				int sub_chunk2_size;
			} header;

			const int data_size = static_cast<int>(m_FileOutput.size()) * 2;
			header.chunk_size = 36 + data_size;
			header.sub_chunk1_size = 16;
			header.sub_chunk2_size = data_size;

			fwrite(&header, sizeof(wave_header), 1, file);

			for (short sample : m_FileOutput)
				fwrite(&sample, 2, 1, file);

			fclose(file);
		}

		m_FileOutput.clear();
		m_FileName.clear();
	}

	bool SIDProxy::IsRecordingToFile() const
	{
		return !m_FileName.empty();
	}

	//------------------------------------------------------------------------------------------------------------

	void SIDProxy::Reset()
	{
		FOUNDATION_ASSERT(m_pSID != nullptr);

		m_pSID->reset();
	}

	int SIDProxy::Clock(int& nDeltaCycles, short* pBuffer, int nBufferSize)
	{
		FOUNDATION_ASSERT(m_pSID != nullptr);

		// Cast to reSid type
		unsigned int nInternalDeltaCycles = static_cast<unsigned int>(nDeltaCycles);

		// Clock
		int nSamplesWritten = m_pSID->clock(nInternalDeltaCycles, pBuffer/*nBufferSize*/);

		// Overwrite with sine wave to test output consistency
//		for (int i = 0; i < nSamplesWritten; ++i)
//		{
//			float r = (static_cast<float>(m_SampleCounter) * 2.0f * 3.1416f) / 25.0f;
//			short v = static_cast<short>(std::sinf(r) * 65535.0f / 4.0f);
//		
//			pBuffer[i] = v;
//		
//			m_SampleCounter++;
//		}

		if (IsRecordingToFile())
		{
			m_FileOutput.push_back(0);
			for (int i = 0; i < nSamplesWritten; ++i)
				m_FileOutput.push_back(pBuffer[i]);
		}

		// Cast back to int
		nDeltaCycles = static_cast<int>(nInternalDeltaCycles);

		// Return number of samples written!
		return nSamplesWritten;
	}

	void SIDProxy::Write(unsigned char ucReg, unsigned char ucValue)
	{
		FOUNDATION_ASSERT(m_pSID != nullptr);
		m_pSID->write(static_cast<int>(ucReg), static_cast<unsigned char>(ucValue));
	}
}
