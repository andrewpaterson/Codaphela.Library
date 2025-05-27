#ifndef __INTERNAL_FIRST_H__
#define __INTERNAL_FIRST_H__
#include "DataOperation.h"


class CInternalFirst : public CDataOperation
{
CONSTRUCTABLE(CInternalFirst);
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __INTERNAL_FIRST_H__

