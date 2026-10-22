#ifndef __DONE_INSTRUCTION_IF_SAME_PAGE_H__
#define __DONE_INSTRUCTION_IF_SAME_PAGE_H__
#include "W65Operation.h"


class CW65DoneInstructionIfSamePage : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __DONE_INSTRUCTION_IF_SAME_PAGE_H__

