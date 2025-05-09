#ifndef __READ_PROGRAM_BANK_H__
#define __READ_PROGRAM_BANK_H__
#include "DataOperation.h"


class CReadProgramBank : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __READ_PROGRAM_BANK_H__

