#ifndef __INTERNAL_FIRST_H__
#define __INTERNAL_FIRST_H__
#include "W65DataOperation.h"


class CW65InternalFirst : public CW65DataOperation
{
CONSTRUCTABLE(CW65InternalFirst);
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __INTERNAL_FIRST_H__

