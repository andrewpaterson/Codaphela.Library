#ifndef __READ_PROGRAM_COUNTER_LOW_H__
#define __READ_PROGRAM_COUNTER_LOW_H__
#include "DataOperation.h"


class CReadProgramCounterLow : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_PROGRAM_COUNTER_LOW_H__

