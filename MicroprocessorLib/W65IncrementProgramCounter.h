#ifndef __INCREMENT_PROGRAM_COUNTER_H__
#define __INCREMENT_PROGRAM_COUNTER_H__
#include "W65Operation.h"


class CW65IncrementProgramCounter : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __INCREMENT_PROGRAM_COUNTER_H__

