#ifndef __NOTE_FOUR_H__
#define __NOTE_FOUR_H__
#include "W65Operation.h"


//Note 4: add 1 cycle for indexing across a page boundary, or a write, or X=0.
class CW65NoteFour : public CW65Operation
{
protected:
	bool	mbIndexY;
	bool	mbNextWillRead;

public:
	void	Init(bool bIndexY, bool bNextWillRead);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	bool    IsConditional(void) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_FOUR_H__

