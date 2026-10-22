#ifndef __READ_PROGRAM_BANK_H__
#define __READ_PROGRAM_BANK_H__
#include "W65DataOperation.h"


class CW65ReadProgramBank : public CW65DataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_PROGRAM_BANK_H__

