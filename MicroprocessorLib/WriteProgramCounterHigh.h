#ifndef __WRITE_PROGRAM_COUNTER_HIGH_H__
#define __WRITE_PROGRAM_COUNTER_HIGH_H__
#include "DataOperation.h"


class CWriteProgramCounterHigh : public CDataOperation
{
public:
	void	Init(void);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROGRAM_COUNTER_HIGH_H__

