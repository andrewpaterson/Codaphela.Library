#ifndef __DONE_INSTRUCTION_IF_EMULATION_H__
#define __DONE_INSTRUCTION_IF_EMULATION_H__
#include "Operation.h"


class CDoneInstructionIfEmulation : public COperation
{
public:
	void    Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void    Print(CChars* psz) override;
};


#endif // __DONE_INSTRUCTION_IF_EMULATION_H__
