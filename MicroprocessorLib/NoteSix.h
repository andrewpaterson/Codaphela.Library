#ifndef __NOTE_SIX_H__
#define __NOTE_SIX_H__
#include "Operation.h"


class CNoteSix : public COperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;

	bool    MustExecute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;
};


#endif // __NOTE_SIX_H__

