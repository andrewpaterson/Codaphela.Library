#ifndef __WRITE_ABSOLUTE_ADDRESS_HIGH_H__
#define __WRITE_ABSOLUTE_ADDRESS_HIGH_H__
#include "W65DataOperation.h"


class CW65WriteAbsoluteAddressHigh : public CW65DataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WRITE_ABSOLUTE_ADDRESS_HIGH_H__

