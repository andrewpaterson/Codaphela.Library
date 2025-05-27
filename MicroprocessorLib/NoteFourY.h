#ifndef __NOTE_FOUR_Y_H__
#define __NOTE_FOUR_Y_H__
#include "Operation.h"


class CNoteFourY : public COperation
{
protected:
	bool mbNextWillRead;

public:
	void	Init(bool bNextWillRead);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_FOUR_Y_H__

