#ifndef __READ_ABSOLUTE_ADDRESS_HIGH_H__
#define __READ_ABSOLUTE_ADDRESS_HIGH_H__
#include "W65DataOperation.h"


class CW65ReadAbsoluteAddressHigh : public CW65DataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock, bool bnotVectorPull);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_ABSOLUTE_ADDRESS_HIGH_H__

