#ifndef __DONE_INSTRUCTION_IF_16BIT_H__
#define __DONE_INSTRUCTION_IF_16BIT_H__
#include "W65WidthFromRegister.h"
#include "W65Operation.h"


class CW65DoneInstructionIf16Bit : public CW65Operation
{
protected:
	EWidthFromRegister  meWidth;

public:
	void    Init(EWidthFromRegister eWidth);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
	size	GetDone16(void) override;
};


#endif // __DONE_INSTRUCTION_IF_16BIT_H__

