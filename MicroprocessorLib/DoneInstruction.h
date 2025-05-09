#ifndef __DONE_INSTRUCTION_H__
#define __DONE_INSTRUCTION_H__
#include "Operation.h"


class CDoneInstruction : public COperation
{
public:
	void	Init(void);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
	size	GetDone8(void) override;
	size	GetDone16(void) override;
};


#endif // __DONE_INSTRUCTION_H__

