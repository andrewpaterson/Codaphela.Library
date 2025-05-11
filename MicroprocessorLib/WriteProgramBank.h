#ifndef __WRITE_PROGRAM_BANK_H__
#define __WRITE_PROGRAM_BANK_H__
#include "DataOperation.h"


class CWriteProgramBank : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_PROGRAM_BANK_H__

