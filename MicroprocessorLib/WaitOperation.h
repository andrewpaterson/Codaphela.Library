#ifndef __WAIT_OPERATION_H__
#define __WAIT_OPERATION_H__
#include "DataOperation.h"


class CWaitOperation : public CDataOperation
{
public:
	void	Init(void);
	void	Execute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __WAIT_OPERATION_H__

