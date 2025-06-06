#ifndef __READ_DATA_LOW_H__
#define __READ_DATA_LOW_H__
#include "DataOperation.h"


class CReadDataLow : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_DATA_LOW_H__

