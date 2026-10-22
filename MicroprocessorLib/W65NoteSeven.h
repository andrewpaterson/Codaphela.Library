#ifndef __NOTE_SEVEN_H__
#define __NOTE_SEVEN_H__
#include "W65Operation.h"


class CW65NoteSeven : public CW65Operation
{
public:
	void	Init(void);

	void	Execute(CW65CCore* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65CCore* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_SEVEN_H__

