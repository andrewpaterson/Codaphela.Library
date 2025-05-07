#ifndef __READ_ABSOLUTE_ADDRESS_HIGH_H__
#define __READ_ABSOLUTE_ADDRESS_HIGH_H__
#include "DataOperation.h"


class CReadAbsoluteAddressHigh : public CDataOperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __READ_ABSOLUTE_ADDRESS_HIGH_H__

