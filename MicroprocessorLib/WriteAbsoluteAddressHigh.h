#ifndef __WRITE_ABSOLUTE_ADDRESS_HIGH_H__
#define __WRITE_ABSOLUTE_ADDRESS_HIGH_H__
#include "DataOperation.h"


class CWriteAbsoluteAddressHigh : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_ABSOLUTE_ADDRESS_HIGH_H__

