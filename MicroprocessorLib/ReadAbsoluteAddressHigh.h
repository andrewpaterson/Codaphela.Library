#ifndef __READ_ABSOLUTE_ADDRESS_HIGH_H__
#define __READ_ABSOLUTE_ADDRESS_HIGH_H__
#include "DataOperation.h"


class CReadAbsoluteAddressHigh : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock, bool bnotVectorPull);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __READ_ABSOLUTE_ADDRESS_HIGH_H__

