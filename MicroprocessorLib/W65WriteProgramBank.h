#ifndef __WRITE_PROGRAM_BANK_H__
#define __WRITE_PROGRAM_BANK_H__
#include "W65DataOperation.h"


class CW65WriteProgramBank : public CW65DataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROGRAM_BANK_H__

