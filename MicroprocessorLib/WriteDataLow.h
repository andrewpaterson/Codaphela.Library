#ifndef __WRITE_DATA_LOW_H__
#define __WRITE_DATA_LOW_H__
#include "DataOperation.h"


class CWriteDataLow : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_DATA_LOW_H__

