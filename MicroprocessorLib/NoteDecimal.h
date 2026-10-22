#ifndef __NOTE_DECIMAL_H__
#define __NOTE_DECIMAL_H__
#include "Operation.h"


class CNoteDecimal : public COperation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_DECIMAL_H__

