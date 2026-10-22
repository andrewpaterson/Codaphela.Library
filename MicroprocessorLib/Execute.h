#ifndef __EXECUTE_H__
#define __EXECUTE_H__
#include "W65CFunc.h"
#include "Operation.h"


class CExecute : public COperation
{
protected:
	CW65CFunc	mfFunction;
	bool			mbInitialSide;

public:
	void	Init(CW65CFunc fFunction, bool bInitialSide);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __EXECUTE_H__

