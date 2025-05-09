#ifndef __READ_ABSOLUTE_ADDRESS_LOW_H__
#define __READ_ABSOLUTE_ADDRESS_LOW_H__
#include "DataOperation.h"


class CReadAbsoluteAddressLow : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock, bool bNotVectorPull);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __READ_ABSOLUTE_ADDRESS_LOW_H__

