#ifndef __SET_PROGRAM_BANK_H__
#define __SET_PROGRAM_BANK_H__
#include "Operation.h"


class CSetProgramBank : public COperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_BANK_H__

