#ifndef __WAIT_OPERATION_H__
#define __WAIT_OPERATION_H__
#include "DataOperation.h"


class CWaitOperation : public CDataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WAIT_OPERATION_H__

