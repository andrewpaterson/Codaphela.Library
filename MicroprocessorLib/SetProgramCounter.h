#ifndef __SET_PROGRAM_COUNTER_H__
#define __SET_PROGRAM_COUNTER_H__
#include "Operation.h"


class CSetProgramCounter : public COperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_COUNTER_H__

