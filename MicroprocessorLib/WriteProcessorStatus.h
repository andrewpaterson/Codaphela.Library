#ifndef __WRITE_PROCESSOR_STATUS_H__
#define __WRITE_PROCESSOR_STATUS_H__
#include "DataOperation.h"


class CWriteProcessorStatus : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROCESSOR_STATUS_H__

