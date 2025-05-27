#ifndef __NOTE_THREE_H__
#define __NOTE_THREE_H__
#include "Operation.h"


class CNoteThree : public COperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	void	Print(CChars* psz) override;
};


#endif // __NOTE_THREE_H__

