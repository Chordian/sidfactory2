#if !defined(__ICPUWRITECALLBACK_H__)
#define __ICPUWRITECALLBACK_H__

class ICPUWriteCallback
{
public:
	ICPUWriteCallback() { }
	virtual ~ICPUWriteCallback() { }

	virtual void Write(unsigned short usAddress, unsigned char ucVal, int iCycle) = 0;
};


#endif //__ICPUWRITECALLBACK_H__