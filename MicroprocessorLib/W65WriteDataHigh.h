#ifndef __WRITE_DATA_HIGH_H__
#define __WRITE_DATA_HIGH_H__
#include "W65DataOperation.h"


class CW65WriteDataHigh : public CW65DataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_DATA_HIGH_H__

