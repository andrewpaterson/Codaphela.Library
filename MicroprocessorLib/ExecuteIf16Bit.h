#ifndef __EXECUTE_IF_16_BIT_H__
#define __EXECUTE_IF_16_BIT_H__
#include "WidthFromRegister.h"
#include "W65C816Func.h"
#include "Operation.h"


class CExecuteIf16Bit : public COperation
{
protected:
	CW65C816Func		mfFunction;
	EWidthFromRegister  meWidth;

public:
	void	Init(CW65C816Func fFunction, EWidthFromRegister eWidth);

	void	Execute(CW65C816* pcCPU) override;

	void	Print(CChars* psz) override;

};


#endif // __EXECUTE_IF_16_BIT_H__

