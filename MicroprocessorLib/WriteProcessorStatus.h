#ifndef __WRITE_PROCESSOR_STATUS_H__
#define __WRITE_PROCESSOR_STATUS_H__
#include "DataOperation.h"


class CWriteProcessorStatus : public CDataOperation
{
protected:
	bool	mbBreak;

public:
	void	Init(void);
	void	Init(bool bBreak);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROCESSOR_STATUS_H__

