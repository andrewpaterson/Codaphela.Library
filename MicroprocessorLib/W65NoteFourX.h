#ifndef __NOTE_FOUR_X_H__
#define __NOTE_FOUR_X_H__
#include "W65Operation.h"


class CW65NoteFourX : public CW65Operation
{
protected:
	bool mbNextWillRead;

public:
	void	Init(bool bNextWillRead);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_FOUR_X_H__

