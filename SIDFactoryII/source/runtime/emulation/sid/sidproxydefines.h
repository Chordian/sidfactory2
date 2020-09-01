#if !defined(__SIDPROXYDEFINES_H__)
#define __SIDPROXYDEFINES_H__

namespace Emulation
{
	enum SIDModel : int
	{
		SID_MODEL_6581,
		SID_MODEL_8580
	};

	enum SIDEnvironment : int
	{
		SID_ENVIRONMENT_PAL,
		SID_ENVIRONMENT_NTSC
	};

	enum SIDSampleMethod : int
	{
		SID_SAMPLE_METHOD_FAST,
		SID_SAMPLE_METHOD_INTERPOLATE,
		SID_SAMPLE_METHOD_RESAMPLE_INTERPOLATE, 
		SID_SAMPLE_METHOD_RESAMPLE_FAST
	};

	struct SIDConfiguration
	{
		SIDModel m_eModel;
		SIDEnvironment m_eEnvironment;
		SIDSampleMethod m_eSampleMethod;
		int m_nSampleFrequency;

		// Default constructor
		SIDConfiguration()
			: m_eModel(SID_MODEL_6581)
			, m_eEnvironment(SID_ENVIRONMENT_PAL)
			, m_eSampleMethod(SID_SAMPLE_METHOD_INTERPOLATE)
			, m_nSampleFrequency(44100)
		{

		}
	};
}


#endif //__SIDPROXYDEFINES_H__