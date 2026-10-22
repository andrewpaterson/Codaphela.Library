#ifndef __NOTE_ONE_H__
#define __NOTE_ONE_H__
#include "WidthFromRegister.h"
#include "Operation.h"


class CNoteOne : public COperation
{
protected:
	EWidthFromRegister meWidth;

public:
	void	Init(EWidthFromRegister eWidth);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_ONE_H__

