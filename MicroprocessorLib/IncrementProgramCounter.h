#ifndef __INCREMENT_PROGRAM_COUNTER_H__
#define __INCREMENT_PROGRAM_COUNTER_H__
#include "Operation.h"


class CIncrementProgramCounter : public COperation
{
public:
	void	Init(void);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __INCREMENT_PROGRAM_COUNTER_H__

