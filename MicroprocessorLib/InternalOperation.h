#ifndef __INTERNAL_OPERATION_H__
#define __INTERNAL_OPERATION_H__
#include "DataOperation.h"


class CInternalOperation : public CDataOperation
{
public:
	void	Init(NotMemoryLock bNotMemoryLock);
	void	Init(bool bValidProgramAddress, bool bValidDataAddress, NotMemoryLock bNotMemoryLock);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __INTERNAL_OPERATION_H__

