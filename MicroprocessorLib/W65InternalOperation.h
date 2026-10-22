#ifndef __INTERNAL_OPERATION_H__
#define __INTERNAL_OPERATION_H__
#include "W65DataOperation.h"


class CW65InternalOperation : public CW65DataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);
	void	Init(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __INTERNAL_OPERATION_H__

