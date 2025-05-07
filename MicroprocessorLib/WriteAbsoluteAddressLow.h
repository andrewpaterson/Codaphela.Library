#ifndef __WRITE_ABSOLUTE_ADDRESS_LOW_H__
#define __WRITE_ABSOLUTE_ADDRESS_LOW_H__
#include "DataOperation.h"


class CWriteAbsoluteAddressLow : public CDataOperation
{
public:
	void Execute(CW65C816* pcCPU) override;
	void Print(CChars* psz) override;
};


#endif // __WRITE_ABSOLUTE_ADDRESS_LOW_H__

