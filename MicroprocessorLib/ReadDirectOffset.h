#ifndef __READ_DIRECT_OFFSET_H__
#define __READ_DIRECT_OFFSET_H__
#include "DataOperation.h"


class CReadDirectOffset : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_DIRECT_OFFSET_H__

