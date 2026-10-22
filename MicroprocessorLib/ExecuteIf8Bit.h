#ifndef __EXECUTE_IF_8_BIT_H__
#define __EXECUTE_IF_8_BIT_H__
#include "WidthFromRegister.h"
#include "W65CFunc.h"
#include "Operation.h"


class CExecuteIf8Bit : public COperation
{
protected:
	CW65CFunc		mfFunction;
	EWidthFromRegister  meWidth;
	bool				mbInitialSide;

public:
	void	Init(CW65CFunc fFunction, EWidthFromRegister eWidth, bool bInitialSide);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __EXECUTE_IF_8_BIT_H__

