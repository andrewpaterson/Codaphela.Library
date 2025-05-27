#ifndef __NOTE_TWO_H__
#define __NOTE_TWO_H__
#include "Operation.h"


class CNoteTwo : public COperation
{
public:
	void	Init(void);

	void	Execute(CW65C816* pcCPU) override;
	bool	IsInitialSide(void) override;

	bool    MustExecute(CW65C816* pcCPU) override;
	void	Print(CChars* psz) override;

};


#endif // __NOTE_TWO_H__

