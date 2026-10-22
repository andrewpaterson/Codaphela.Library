#ifndef __WRITE_PROGRAM_COUNTER_LOW_H__
#define __WRITE_PROGRAM_COUNTER_LOW_H__
#include "DataOperation.h"


class CWriteProgramCounterLow : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROGRAM_COUNTER_LOW_H__

