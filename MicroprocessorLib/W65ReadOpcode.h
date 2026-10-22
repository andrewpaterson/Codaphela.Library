#ifndef __READ_OP_CODE_H__
#define __READ_OP_CODE_H__
#include "W65DataOperation.h"


class CW65ReadOpcode : public CW65DataOperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
	size	GetDone8(void) override;
	size	GetDone16(void) override;
	bool	IsFetchOpcode(void) override;
};


#endif // __READ_OP_CODE_H__

