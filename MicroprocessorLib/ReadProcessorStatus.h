#ifndef __READ_PROCESSOR_STATUS_H__
#define __READ_PROCESSOR_STATUS_H__
#include "DataOperation.h"


class CReadProcessorStatus : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_PROCESSOR_STATUS_H__

