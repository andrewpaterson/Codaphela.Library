#ifndef __DONE_INSTRUCTION_IF_BINARY_H__
#define __DONE_INSTRUCTION_IF_BINARY_H__
#include "W65Operation.h"


class CW65DoneInstructionIfBinary : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __DONE_INSTRUCTION_IF_BINARY_H__

