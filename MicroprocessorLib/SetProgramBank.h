#ifndef __SET_PROGRAM_BANK_H__
#define __SET_PROGRAM_BANK_H__
#include "Operation.h"


class CSetProgramBank : public COperation
{
protected:
	uint8	muiBank;

public:
	void	Init(uint8 uiBank);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_BANK_H__

