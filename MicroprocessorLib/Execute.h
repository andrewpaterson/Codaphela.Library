#ifndef __EXECUTE_H__
#define __EXECUTE_H__
#include "W65C816Func.h"
#include "Operation.h"


class CExecute : public COperation
{
protected:
	CW65C816Func	mfFunction;
	bool			mbInitialSide;

public:
	void	Init(CW65C816Func fFunction, bool bInitialSide);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __EXECUTE_H__

