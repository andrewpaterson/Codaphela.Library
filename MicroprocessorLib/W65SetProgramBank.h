#ifndef __SET_PROGRAM_BANK_H__
#define __SET_PROGRAM_BANK_H__
#include "W65Operation.h"


class CW65SetProgramBank : public CW65Operation
{
protected:
	uint8	muiBank;

public:
	void	Init(uint8 uiBank);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __SET_PROGRAM_BANK_H__

