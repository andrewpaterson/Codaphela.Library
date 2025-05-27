#ifndef __EXECUTE_IF_8_BIT_H__
#define __EXECUTE_IF_8_BIT_H__
#include "WidthFromRegister.h"
#include "W65C816Func.h"
#include "Operation.h"


class CExecuteIf8Bit : public COperation
{
protected:
	CW65C816Func		mfFunction;
	EWidthFromRegister  meWidth;
	bool				mbInitialSide;

public:
	void	Init(CW65C816Func fFunction, EWidthFromRegister eWidth, bool bInitialSide);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __EXECUTE_IF_8_BIT_H__

