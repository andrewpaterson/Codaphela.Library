#ifndef __DONE_INSTRUCTION_IF_SAME_PAGE_H__
#define __DONE_INSTRUCTION_IF_SAME_PAGE_H__
#include "Operation.h"


class CDoneInstructionIfSamePage : public COperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __DONE_INSTRUCTION_IF_SAME_PAGE_H__

