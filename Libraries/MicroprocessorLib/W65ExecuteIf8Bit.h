#ifndef __EXECUTE_IF_8_BIT_H__
#define __EXECUTE_IF_8_BIT_H__
#include "W65WidthFromRegister.h"
#include "W65CFunc.h"
#include "W65Operation.h"


class CW65ExecuteIf8Bit : public CW65Operation
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

