#ifndef __DONE_INSTRUCTION_H__
#define __DONE_INSTRUCTION_H__
#include "W65Operation.h"


class CW65DoneInstruction : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) const override;

	void	Print(CChars* psz) override;
	size	GetDone8(void) const override;
	size	GetDone16(void) const override;
};


#endif // __DONE_INSTRUCTION_H__

