#ifndef __WRITE_ABSOLUTE_ADDRESS_LOW_H__
#define __WRITE_ABSOLUTE_ADDRESS_LOW_H__
#include "DataOperation.h"


class CWriteAbsoluteAddressLow : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_ABSOLUTE_ADDRESS_LOW_H__

