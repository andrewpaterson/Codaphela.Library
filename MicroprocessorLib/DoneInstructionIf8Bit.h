#ifndef __DONE_INSTRUCTION_IF_8BIT_H__
#define __DONE_INSTRUCTION_IF_8BIT_H__
#include "WidthFromRegister.h"
#include "Operation.h"


class CDoneInstructionIf8Bit : public COperation
{
protected:
	EWidthFromRegister  meWidth;

public:
    void    Init(EWidthFromRegister eWidth);
	void    Execute(CW65C816* pcCPU) override;
	void    Print(CChars* psz) override;
    size	GetDone8(void) override;
};


#endif // __DONE_INSTRUCTION_IF_8BIT_H__

