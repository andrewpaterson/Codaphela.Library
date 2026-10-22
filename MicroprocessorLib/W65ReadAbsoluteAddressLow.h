#ifndef __READ_ABSOLUTE_ADDRESS_LOW_H__
#define __READ_ABSOLUTE_ADDRESS_LOW_H__
#include "W65DataOperation.h"


class CW65ReadAbsoluteAddressLow : public CW65DataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock, bool bNotVectorPull);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __READ_ABSOLUTE_ADDRESS_LOW_H__

