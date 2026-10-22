#ifndef __WAIT_OPERATION_H__
#define __WAIT_OPERATION_H__
#include "W65DataOperation.h"


class CW65WaitOperation : public CW65DataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __WAIT_OPERATION_H__

