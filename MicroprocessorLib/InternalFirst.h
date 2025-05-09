#ifndef __INTERNAL_FIRST_H__
#define __INTERNAL_FIRST_H__
#include "DataOperation.h"


class CInternalFirst : public CDataOperation
{
public:
	void	Init(void);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __INTERNAL_FIRST_H__

