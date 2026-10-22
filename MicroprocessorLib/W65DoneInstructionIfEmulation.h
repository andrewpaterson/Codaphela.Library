#ifndef __DONE_INSTRUCTION_IF_EMULATION_H__
#define __DONE_INSTRUCTION_IF_EMULATION_H__
#include "W65Operation.h"


class CW65DoneInstructionIfEmulation : public CW65Operation
{
public:
	void    Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void    Print(CChars* psz) override;
};


#endif // __DONE_INSTRUCTION_IF_EMULATION_H__
